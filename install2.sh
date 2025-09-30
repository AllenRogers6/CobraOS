## add checks for gcc, make, etc
deps=("gcc" "make" "git" "mtools" "xorriso")

for dep in "${deps[@]}"; do
	if ! command -v "$dep" &>/dev/null; then
		echo "One or more dependencies are missing."
	fi
done

read -p "Install? (y/n): " choice

if [ "$choice" == "n" ]; then
	exit 1
fi

os=$(lsb_release -is | awk '{print $0}' 2>/dev/null)

if [[ "$os" == "Debian" || "$os" == "Ubuntu" ]]; then
	#sudo apt update -y
	sudo apt install -y "${deps[@]}" || {
		echo "Packages not found. Leaving..."
		exit 1
	}
	#sudo autoremove -y
elif [ "$os" == "Archlinux" ]; then
	sudo pacman -Syu
	sudo pacman -S --needed "${deps[@]}"
fi
