for dir in */; do
  if [ -d "${dir}" ]; then
    if [ -e "${dir}/run.sh" ]; then
      echo "$dir"
      cd "${dir}"
      ./run.sh
      cd ..
    fi
  fi
done
