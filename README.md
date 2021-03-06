Skia is a complete 2D graphic library for drawing Text, Geometries, and Images.

See full details, and build instructions, at https://skia.org.

SVG2渲染库

```
https://www.w3.org/TR/SVG11/
https://www.w3.org/TR/SVG2/
```

Lottie库

```
https://github.com/airbnb/lottie-android
https://github.com/airbnb/lottie-ios
# docs含有json的解释
https://github.com/airbnb/lottie-web
```

Blink项目

```
https://chromium.googlesource.com/chromium/blink/
```

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

#### 使用跨平台Conan工具

安装conan

```
pip3 install conan
source ~/.profile
```

查看远程仓库

```
conan remote list
```

配置仓库

```
conan-center: https://conan.bintray.com [Verify SSL: True]
upload_repo: https://api.bintray.com/conan/bincrafters/public-conan [Verify SSL: True]
conan-community: https://api.bintray.com/conan/conan-community/conan [Verify SSL: True]
nonstd-lite: https://api.bintray.com/conan/martinmoene/nonstd-lite [Verify SSL: True]
```

```
conan remote add upload_repo https://api.bintray.com/conan/bincrafters/public-conan [Verify SSL: True]
```

生成Linux包

```
conan create . user/stable --profile profile/Linux-x86_64-release --build missing
```

生成Android包

```
conan create . user/stable --profile profile/Android-armv8-release --build missing
```

#### 打包Android AAR库文件

示范步骤，相关代码在 `android/` 目录中。

1. 修改 `build.gradle` 文件，使Android工程能够找到c/c++库和CMakeLists.txt文件。

2. 将c/c++库放入到 `libs` 目录，将头文件放置到 `include` 目录。

3. 编写java层的接口供app调用。

4. 编写JNI程序调用c/c++库。

5. 打包生成AAR库文件。


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