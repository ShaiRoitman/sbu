git status | grep modi | clu -a 2 | clu --format "git add %%%%1" | clu --system
git commit -m "not yet ready"
