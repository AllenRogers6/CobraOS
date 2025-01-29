binFiles=$(find . -name "*.bin")
isoFiles=$(find . -name "*.iso")
if [ -n "$binFiles" ]; then
  echo "Deleting bin files..."
  echo "$binFiles" | xargs rm -rf
else
  echo "No bin files found."
fi

if [ -n "$isoFiles" ]; then
  echo "Deleting iso files..."
  echo "$isoFiles" | xargs rm -rf
else
  echo "No iso files found."
fi
