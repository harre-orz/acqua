# ドキュメント

## はじめに
  acqua library (以下、acqua)は、C++ のヘッダーファイルのみで構成されたライブラリです。
  C++11 に準拠したコンパイラと Boost C++ Library が必要ですが、それ以外に依存しているライブラリはありません。
  また、すべてのコードがヘッダーファイルにプログラムが書かれているので、簡単にコードを読んで確認することができます。

## ライブラリ一覧

### 例外

acqua::exception::backtrace は、バックトレースを出力するマニピュレータ。 acqua::exception::errinfo_backtrace は boost::exception 用のマニピュレータ。


### コンテナ

acqua::container::sequenced_map クラス。
acqua::container::sequenced_multimap クラス。


### 非同期通信

TCPのサーバを簡単に作れる acqua::asio::simple_server クラス。
IPv4、IPv6の区別ないサーバを簡単に作れる acqua::asio::internet_server クラス。
acqua::asio::server_traits クラスを工夫すれば SSL にも対応可能。

ping を非同期に行う acqua::asio::pinger_v4 クラス。
ping6 を非同期に行う acqua::asio::pinger_v6 クラス。

ファイルの通知を行う acqua::asio::inotify クラス（Linuxのみ対応）。

Ethernetフレームからパケットを取得できる acqua::asio::raw ソケットクラス（Linuxのみ対応）。


### ネットワーク

acqua::network::linklayer_address クラス。
acqua::network::internet4_address クラス。
acqua::network::internet6_address クラス


### ウェブクライアント

acqua::webclient::http_client クラスは、同期もしくは非同期なHTTP通信やHTTPS通信と、同じサーバに対してはキープアライブを用いて、同じコネクションを使い回す。
GETには acqua::webclient::wget 関数を用い、POSTには acqua::webclient::wpost 関数を用いる。
