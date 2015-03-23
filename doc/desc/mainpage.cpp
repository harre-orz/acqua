/*!
  \mainpage ドキュメント

  \section はじめに
  acqua library (以下、acqua)は、C++ のヘッダーファイルのみで構成されたライブラリです。
  C++11 に準拠したコンパイラと Boost C++ Library が必要ですが、それ以外に依存しているライブラリはありません。
  また、すべてのコードがヘッダーファイルにプログラムが書かれているので、簡単にコードを読んで確認することができます。

  \section ライブラリ一覧

  \subsection 例外
  <dl>
  <dt>バックトレース</dt>
  <dd>acqua::exception::backtrace は、バックトレースを出力するマニピュレータ。 acqua::exception::errinfo_backtrace は boost::exception 用のマニピュレータ。</dd>
  </dl>

  \subsection コンテナ
  <dl>
  <dt>挿入順マップ</dt>
  <dd>acqua::container::sequenced_map クラス。 acqua::container::sequenced_multimap クラス。</dd>
  </dl>

  \subsection 非同期通信
  <dl>
  <dt>TCPサーバ</dt>
  <dd>TCPのサーバを簡単に作れる acqua::asio::simple_server クラス。IPv4、IPv6の区別ないサーバを簡単に作れる acqua::asio::internet_server クラス。 acqua::asio::server_traits クラスを工夫すれば SSL にも対応可能。</dd>
  <dt>pingチェック</dt>
  <dd>ping を非同期に行う acqua::asio::pinger_v4 クラス。ping6 を非同期に行う acqua::asio::pinger_v6 クラス。</dd>
  <dt>ファイル通知</dt>
  <dd>ファイルの通知を行う acqua::asio::inotify クラス（Linuxのみ対応）。</dd>
  <dt>RAWソケット</dt>
  <dd>Ethernetフレームからパケットを取得できる acqua::asio::raw ソケットクラス（Linuxのみ対応）。</dd>

  \subsection ネットワーク
  <dl>
  <dt>アドレス</dt>
  <dd>リンクレイヤーアドレスの acqua::network::linklayer_address クラス。IPv4アドレスの acqua::network::internet4_address クラス。IPv6アドレスの acqua::network::internet6_address クラス</dd>

  \subsection ウェブサイト
  <dl>
  <dt>HTTPクライアント</dt>
  <dd>acqua::website::http_client クラスは、同期もしくは非同期なHTTP通信やHTTPS通信と、同じサーバに対してはキープアライブを用いて、同じコネクションを使い回すことができます。GETには acqua::website::wget 関数を用い、POSTには acqua::website::wpost 関数を用います。</dd>
  </dl>
 */
