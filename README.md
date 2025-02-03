# Seam-Carver


A C++ program that implements the seam carving algorithm for content-aware image resizing.

## Dependencies

- OpenCV library (for image processing)


## Features

- Vertical and horizontal seam carving
- Energy map calculation using Sobel operators


## Compilation

```
g++ seam-carver.cpp -o seam -I/usr/local/include/opencv4 -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc
```

## Usage

```
./seam <input_image_path> <target_width> <target_height>
```



## Output

- Displays seam removal process in real-time
- Saves resized image as "output_[width]x[height]_[original_filename]"

## Notes

- Ensure target dimensions are smaller than or equal to the original image dimensions
- The program visualizes each seam removal step
- Press any key to close the final visualization window
- If the target width and target height are too small, images can get distorted and also if images have too many details while constructing energy map the image can have many details and computing seam and removing will introduce image distortions.

