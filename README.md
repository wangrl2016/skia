Skia is a complete 2D graphic library for drawing Text, Geometries, and Images.

See full details, and build instructions, at https://skia.org.

SVG2渲染库

https://www.w3.org/TR/SVG2/

* 同步google远程仓库

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
