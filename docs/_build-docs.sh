#!/usr/bin/env bash

SCRIPT_PATH="$(cd "$(dirname "$0")"; pwd -P)"
MAIN_TITLE="3D Game Shaders For Beginners"
REPO_URL="https://github.com/lettier/3d-game-shaders-for-beginners"
AUTHOR="David Lettier"
CSS="style.css"

for f in $SCRIPT_PATH/../sections/*
do
  echo "$f"
  file=$(basename -- "$f")
  file_name="${file%.*}"
  title=$(echo "$file_name" | sed -r 's/-/ /g' | sed -e 's/\b\(.\)/\u\1/g')
  if [ "$title" == "Ssao" ]
  then
    title="SSAO"
  fi
  if [ "$title" == "Glsl" ]
  then
    title="GLSL"
  fi
  $PANDOC \
    -f gfm \
    -t html5 \
    --highlight-style=breezedark \
    --template=$SCRIPT_PATH/_template.html5 \
    $f \
    --metadata pagetitle="$title | $MAIN_TITLE" \
    --metadata author-meta="$AUTHOR" \
    --metadata css=$CSS \
    -o "$SCRIPT_PATH/$file_name.html"
done

$PANDOC \
  -f gfm \
  -t html5 \
  --highlight-style=breezedark \
  --template=$SCRIPT_PATH/_template.html5 \
  $SCRIPT_PATH/../README.md \
  --metadata pagetitle="$MAIN_TITLE" \
  --metadata author-meta="$AUTHOR" \
  --metadata css=$CSS \
  -o "$SCRIPT_PATH/index.html"

for i in {1..3}
do
  for f in $SCRIPT_PATH/*
  do
    file=$(basename -- "$f")
    file_name="${file%.*}"
    file_ext="${file##*.}"
    if [ "$file_ext" == "html" ]
    then
      echo $f
      sed -i -E 's/href="(sections\/)?([a-z-]+)\.md(.*)"/href="\2\.html\3"/g' $f
      sed -i -E 's/href="\.\.\/README.md"/href="index.html"/g' $f
      sed -i -E 's+<a href="\.\.\/demonstration\/(.*)">+<a href="'$REPO_URL'\/blob\/master\/demonstration\/\1" target="_blank" rel="noopener noreferrer">+g' $f
    fi
  done
done
