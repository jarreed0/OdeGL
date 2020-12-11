echo "update: "
read u
git add build.sh
git add *.h
git add *.cpp
echo "" >> README.md
date >> README.md
echo $u >> README.md
git add README.md
git commit -m $u
git push
