# https://github.com/niladridutt/Diffusion-3D-Features/blob/main/diff3f.py
# https://github.com/niladridutt/Diffusion-3D-Features/blob/main/test_correspondence.ipynb


import torch
import numpy as np
from PIL import Image
from torchvision import transforms as T
import torch.nn.functional as F

device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

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
    uh = 1 if corner_aligned else 1 - (1 / h)
    uw = 1 if corner_aligned else 1 - (1 / w)
    
    if margin > 0:
        uh += (2 / h) * margin
        uw += (2 / w) * margin
        w += margin * 2
        h += margin * 2

    x = torch.linspace(-uw, uw, w)
    y = torch.linspace(-uh, uh, h)

    if jitter is not None:
        x += ((torch.rand_like(x) - 0.5) * 2 / w) * jitter
        y += ((torch.rand_like(y) - 0.5) * 2 / h) * jitter

    x, y = torch.meshgrid(x, y, indexing='ij')
    grid = torch.stack([x, y], dim=-1).permute(1, 0, 2)  # [H, W, 2]
    grid = grid.reshape(1, -1, 2).repeat(batch_size, 1, 1)

    if subsample_to is not None and subsample_to < h * w:
        idx = np.random.choice(grid.shape[1], size=subsample_to, replace=False)
        grid = grid[:, idx]

    if invert_y_axis:
        grid[..., 1] *= -1.0

    return grid

def init_dino(device):
    model = torch.hub.load("facebookresearch/dinov2", "dinov2_vits14")
    return model.to(device).eval()

@torch.no_grad()
def get_dino_features(device, dino_model, or_img, grid, depth_mask=None):
    # Transform and send to GPU
    transform = T.Compose([
        T.Resize((518, 518)),
        T.ToTensor(),
        T.Normalize((0.485, 0.456, 0.406), (0.229, 0.224, 0.225)),
    ])
    img = transform(or_img)[:3].unsqueeze(0).to(device)

    # Extract features from last DINO layer
    features = dino_model.get_intermediate_layers(img, n=1)[0]  # (1, N, 768)
    patch_size = 14
    H_feat = img.shape[2] // patch_size
    W_feat = img.shape[3] // patch_size
    dim = features.shape[-1]

    # Reshape to 2D feature map
    features = features.view(1, H_feat, W_feat, dim).permute(0, 3, 1, 2)  # (1, 768, H', W')

    # Upsample to match grid resolution
    features = F.interpolate(features, size=(grid.shape[1], grid.shape[2]), mode="bilinear", align_corners=False)

    # Optionally half precision (if your model supports it)
    features = features.half()
    grid = grid.half()

    # Sample features at pixel grid locations
    sampled = F.grid_sample(features, grid, align_corners=False)  # (1, 768, 1, H*W)
    sampled = sampled.view(1, dim, -1)

    # Normalize feature vectors
    sampled = F.normalize(sampled, dim=1)

    return sampled

def main():
    # Load model
    dino = init_dino(device)

    # Load and prepare image
    image_path = "./temp/depth.png"
    image = Image.open(image_path).convert("RGB")
    width, height = image.size

    # Create normalized grid (same resolution as image)
    grid = arange_pixels((height, width), invert_y_axis=False)[0].to(device)
    grid = grid.reshape(1, height, width, 2).half()

    # Extract features
    features = get_dino_features(device, dino, image, grid)

    # print("Output features shape:", features.shape)  # [1, 768, H*W]

    # get first 3 features
    feat_vis = features[0, :3, :]
    
    # get 3 averages for 3 slices
    # B, C, N = features.shape  # B=1, C=768, N=H*W
    # slice_size = C // 3
    # slice1 = features[:, :slice_size, :].mean(dim=1)
    # slice2 = features[:, slice_size:2*slice_size, :].mean(dim=1)
    # slice3 = features[:, 2*slice_size:, :].mean(dim=1)
    # feat_vis = torch.cat([slice1, slice2, slice3], dim=0)


    feat_vis = feat_vis.reshape(3, height, width).cpu()
    feat_img = T.ToPILImage()(feat_vis)
    feat_img.save("./temp/feature.png")


if __name__ == "__main__":
    main()
