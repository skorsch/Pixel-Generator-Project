Pixel Art Generator

Building/Install Instructions: As outlined in section 4.3 of the project assignment PDF - use cmake building commands in the top level of this directory

Let $TOP_DIR denote the directory containing this README file.
Let $INSTALL_DIR denote the directory into which this
software is to be installed.
To build and install the software, use the commands:
    cd $TOP_DIR
    cmake -H. -Btmp_cmake -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR
    cmake --build tmp_cmake --clean-first --target install

To run a demonstration, use the commands:
    $INSTALL_DIR/bin/demo

eg. If the $INSTALL_DIR is ../build
$ cmake -H. -Btmp_cmake -DCMAKE_INSTALL_PREFIX=../build
$ cmake --build tmp_cmake --clean-first --target install
$ ../build/bin/demo 

Running the software:
    Once the executable has been built and installed, it requires some user input through command line arguments in the following style:
        $ <path>/pixel_art_gen <input_file_path> <output_file_path> < <transformations> >

        Where:
            <input_file_path> is the path and file name for the photo you wish to transform -- according to the OpenCV API .bmp, .dib, .jpeg, .jpg, .jpe. .jp2, .png, .pbm, .pgm, .ppm, .sr, .ras, .tiff, .tif are supported, otherwise it will fail to find the file

            <output_file_path> is the path and file name for the photo output you wish to generate -- all input file types are also supported for output, and the input type can be different from the output type (eg. in.png -> out.jpeg)

            < <transformations> > is a list of up to three different transformations that can be applied to an image (non-repeating), can be given in any order
                - 1 or none of a <SIZE REDUCTION ALGORITHM>
                    -- choose one of: nearest_neighbor, bilinear, bicubic, and lanczos along with a dash and a numerical value that indicates the width of the output (ratio will be kept)
                    -- note: upsizing will also work but usually not ideal for pixel work
                    -- eg. nearest_neighber-200
                - 1 or none of a <COLOR REDUCTION ALGORITHM>
                    -- choose one of: uniform, popularity, median_cut, and kmeans along with a dash and the numbers of colors in the final image
                    -- num_colors = 2 is a special case for black and white
                    -- note: uniform may result in less colors than requested
                    -- eg. uniform-8
                - 1 or none of a <DITHERING ALGORITHM>
                    -- choose one of: bayer or atkinson
                    -- these require a color reduction algorithm to be given, but if not a default of kmeans-8 will be used

        
        eg. If in the top directory of the $INSTALL_DIR
        $ ./pixel_art_gen ./bin/demo_assets/demo_1.jpg ./outt.png bayer nearest_neighbor-200 kmeans-8

    The demo file also has more example arguments