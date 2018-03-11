[English](./README.md)
# Fafnir

Clang MSBuild toolset for Visual Studio 2017

## 説明

Visual Studio 2017およびClang for Windowsがインストールされた環境で、MSBuildから利用できるツールセットを追加するツールです。

## 前提条件

* Visual Studio 2017
* Clang For Windows  
  LLVMの公式サイトからインストーラーを[ダウンロード](http://releases.llvm.org/download.html)できます。  
  [スナップショット ビルド](http://llvm.org/builds/)を利用することや、自分でビルドしたものを使うこともできます。

## 使い方

Clang for Windowsのホストアーキテクチャに合わせたビルドをダウンロードしてください。

### インストール

install.batを実行し、対話式のインストーラーでLLVMのパスとインストールするツールセット名を指定してください。  
実行には管理者権限が必要です。  
なおCMakeから利用したい場合、ツールセット名は`v[0-9]+_clang_.*`にマッチする名前にしないといけません。  

### Visual Studio IDE 上から使う場合

プロジェクトのプロパティを開いて、左側リストから \[構成プロパティ -> 全般\] を選び、 \[全般\] グループにある \[プラットフォーム ツールセット\] を開き、インストール時に指定した名前（デフォルトはv100_clang_fafnir）を選んでください。  
後は普通にビルドすればLLVMのclangでコンパイルされます。

### CMakeから使う場合

cmakeコマンドを実行する時に `-G` オプションで `"Visual Studio 15 2017"` もしくは `"Visual Studio 15 2017 Win64"` を指定し、 `-T` オプションでインストール時に指定した名前を選んでください。

## ビルド方法

今のところ、Fafnirをビルドするには、Fafnirを利用してインストールしたツールセットが必要です。  
また、LLVMはC++17に対応したものが必要です。（リリース物は[LLVM Snapshot Build](http://llvm.org/builds/)を利用しています。
また、ビルドシステムはCMakeに依存しています。

* 64bit版ビルド
```
cmake -G "Visual Studio 15 2017 Win64" -T <インストール済みFafnirツールセット> -DCMAKE_INSTALL_PREFIX=<インストール先のパス>
cmake --build . --config Release --target INSTALL
```

* 32bit版ビルド
```
cmake -G "Visual Studio 15 2017" -T <インストール済みFafnirツールセット> -DCMAKE_INSTALL_PREFIX=<インストール先のパス>
cmake --build . --config Release --target INSTALL
```

上記のコマンドで、インストール先にリリース物と同じ構成でファイルが配置されます。
