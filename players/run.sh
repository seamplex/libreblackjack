for dir in */; do
  if [ -d "${dir}" ]; then
    echo "$dir"
    cd "${dir}"
    ./run.sh
    cd ..
  fi
done
