# Fafnir

Clang MSBuild toolset for Visual Studio 2017

## 説明

* LLVM公式バイナリに含まれているツールセットファイルが古い＆32bit版しか用意されていない  

かつ  

* Microsoftが試験実装したClang/C2のバージョンが古い  

ため、最新のLLVMビルドをMSBuildから使えるツールセットを追加しようというプロジェクトです。

## 使い方

install.batを実行し、LLVMのパスとインストールするツールセット名を指定してください。  
なおCMakeから利用したい場合、ツールセット名は`v[0-9]+_clang_.*`にマッチする名前にしないといけません。  
