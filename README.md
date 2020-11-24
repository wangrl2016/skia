Skia is a complete 2D graphic library for drawing Text, Geometries, and Images.

See full details, and build instructions, at https://skia.org.

SVG2渲染库

https://www.w3.org/TR/SVG2/

#### 编译Skia仓库

配置编译工具

```
git clone 'https://chromium.googlesource.com/chromium/tools/depot_tools.git'
export PATH="${PWD}/depot_tools:${PATH}"
```

下载第三方依赖

```
python2 tools/git-sync-deps
```

生成默认GN编译文件

```
bin/gn gen out/default
```

编译指定的`editor`程序

```
ninja -C out/default HelloWorld

or

ninja -C out/default editor
```

运行

```
./out/default/HelloWorld

or

./out/default/editor resources/text/english.txt
```

生成CMake编译文件

```
bin/gn gen out/config --ide=json --json-ide-script=../../gn/gn_to_cmake.py
```

#### 同步google远程仓库

查看配置的远程仓库

```
git remote -v
// origin	https://github.com/wangrl2016/skia (fetch)
// origin	https://github.com/wangrl2016/skia (push)
```

指定远程upstream仓库

```
git remote add upstream https://github.com/google/skia
```

通过`git remote -v`再次查看

```
origin	https://github.com/wangrl2016/skia (fetch)
origin	https://github.com/wangrl2016/skia (push)
upstream	https://github.com/google/skia (fetch)
upstream	https://github.com/google/skia (push)
```

获取远程仓库的更新

```
git fetch upstream
```

切换到本地想要同步的master分支

```
git checkout master
```

合并upstream/master分支进入master分支

```
git merge upstream/master

or

git rebase upstream/master
```

切换python版本

```
update-alternatives --config python
```