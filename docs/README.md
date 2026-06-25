cd /mnt/hgfs/share/Shm
cat > README.md << 'EOF'
# 智能家庭影音系统

## 项目简介
基于 GEC6818 开发板的智能家庭影音系统

## 功能模块
- 音乐播放 (madplay)
- 图片相册 (BMP显示)
- 视频播放 (mplayer)
- 游戏中心 (Piano钢琴)

## 素材说明
音乐、视频、图片素材请自行准备，放置于 resources/ 目录下

## 编译
make clean && make

## 运行
./shm
EOF

git add README.md
git commit -m "docs: 添加 README"
git push
