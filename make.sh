em++ \
  -std=c++11 \
  -Wall \
  -O2 \
  --emrun \
  --preload-file ./data@/ \
  ./main.cpp \
  -o main.html \
  -I /home/gray/big_drive/projects/Urho3D-embuild/include \
  -L /home/gray/big_drive/projects/Urho3D-embuild/lib \
  -lUrho3D -lGL -lGLU -lGLEW

