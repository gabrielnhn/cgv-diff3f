import torch
import PIL

import torch
import numpy as np
from torchvision import transforms as tfs

device = torch.device("cuda") if torch.cuda.is_available() else torch.device("cpu")

def arange_pixels(
    resolution=(128, 128),
    batch_size=1,
    subsample_to=None,
    invert_y_axis=False,
    margin=0,
    corner_aligned=True,
    jitter=None,
):
    h, w = resolution
    n_points = resolution[0] * resolution[1]
    uh = 1 if corner_aligned else 1 - (1 / h)
    uw = 1 if corner_aligned else 1 - (1 / w)
    if margin > 0:
        uh = uh + (2 / h) * margin
        uw = uw + (2 / w) * margin
        w, h = w + margin * 2, h + margin * 2

    x, y = torch.linspace(-uw, uw, w), torch.linspace(-uh, uh, h)
    if jitter is not None:
        dx = (torch.ones_like(x).uniform_() - 0.5) * 2 / w * jitter
        dy = (torch.ones_like(y).uniform_() - 0.5) * 2 / h * jitter
        x, y = x + dx, y + dy
    x, y = torch.meshgrid(x, y)
    pixel_scaled = (
        torch.stack([x, y], -1)
        .permute(1, 0, 2)
        .reshape(1, -1, 2)
        .repeat(batch_size, 1, 1)
    )

    if subsample_to is not None and subsample_to > 0 and subsample_to < n_points:
        idx = np.random.choice(
            pixel_scaled.shape[1], size=(subsample_to,), replace=False
        )
        pixel_scaled = pixel_scaled[:, idx]

    if invert_y_axis:
        pixel_scaled[..., -1] *= -1.0

    return pixel_scaled


def init_dino(device):
    model = torch.hub.load(
        "facebookresearch/dinov2",
        "dinov2_vits14",
    )
    
    model = model.to(device).eval()
    return model

@torch.no_grad
def get_dino_features(device, dino_model, img, grid):
    transform = tfs.Compose(
        [
            tfs.Resize((518, 518)),
            tfs.ToTensor(),
            tfs.Normalize((0.485, 0.456, 0.406), (0.229, 0.224, 0.225)),
        ]
    )
    img = transform(img)[:3].unsqueeze(0).to(device)
    features = dino_model.get_intermediate_layers(img, n=1)[0].half()
    patch_size = 14
    h, w = int(img.shape[2] / patch_size), int(img.shape[3] / patch_size)
    dim = features.shape[-1]
    features = features.reshape(-1, h, w, dim).permute(0, 3, 1, 2)
    features = torch.nn.functional.grid_sample(
        features, grid, align_corners=False
    ).reshape(1, 768, -1)
    features = torch.nn.functional.normalize(features, dim=1)
    return features


reverse = tfs.Compose([
    tfs.ToPILImage()
])

# dino = torch.hub.load('facebookresearch/dino:main', 'dino_resnet50')
dino = init_dino(torch.device("cuda"))

path = "./temp/depth.png"
image = PIL.Image.open(path)
width, height = image.size

grid = arange_pixels((height, width), invert_y_axis=False)[0].to(device).reshape(1, height, width, 2).half()
output = get_dino_features(torch.device("cuda"),dino, image, grid)

print("OUTSHAPE", output.shape)


new_image = reverse(output)
new_image = new_image.resize((width, height))
new_image.save("./temp/dino.png")