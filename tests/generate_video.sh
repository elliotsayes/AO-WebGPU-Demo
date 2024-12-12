#!/bin/bash
ffmpeg -framerate 30 -pattern_type glob -i 'output/frame*.png' \
  -c:v libx264 -pix_fmt yuv420p output/frames.mp4
