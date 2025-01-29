objFiles=$(find . -name "*.o")
if [ -n "$objFiles" ]; then
  echo "Deleting object files..."
  echo "$objFiles" | xargs rm -rf
else
  echo "No object files found."
fi
