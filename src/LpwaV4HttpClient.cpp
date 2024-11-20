#include "LpwaV4HttpClient.h"
#include "LpwaV4Modem.h"

LpwaV4HttpClient::LpwaV4HttpClient(const char *host, uint16_t port, const bool https)
    : _host(host), _port(port), _https(https), _status(-1), _body(""), _requestHeader(""),
      _requestBody("")
{
}

LpwaV4HttpClient::LpwaV4HttpClient(String &host, uint16_t port, const bool https)
    : _host(host), _port(port), _https(https), _status(-1), _body(""), _requestHeader(""),
      _requestBody("")
{
}

LpwaV4HttpClient::~LpwaV4HttpClient()
{
}

/** Delete Session */
void LpwaV4HttpClient::_close()
{
}

/**
 * GETリクエストを送信します。
 * @param path リクエストパス
 * @return ステータスコード (取得に失敗した場合: -1)
 */
int LpwaV4HttpClient::get(const char *path)
{
  return sendRequest(HTTP_REQUEST_METHOD_GET, path);
}

/**
 * POSTリクエストを送信します。
 * @param path リクエストパス
 * @param contentType Content-Type ヘッダー
 * @param body リクエストボディ
 * @return ステータスコード (取得に失敗した場合: -1)
 */
int LpwaV4HttpClient::post(const char *path, const char *contentType,
                           const char *body)
{
  _contentType = contentType;
  setBody(body);
  return sendRequest(HTTP_REQUEST_METHOD_POST, path);
}

/**
 * PUTリクエストを送信します。
 * @param path リクエストパス
 * @param contentType Content-Type ヘッダー
 * @param body リクエストボディ
 * @return ステータスコード (取得に失敗した場合: -1)
 */
int LpwaV4HttpClient::put(const char *path, const char *contentType,
                          const char *body)
{
  _contentType = contentType;
  setBody(body);
  return sendRequest(HTTP_REQUEST_METHOD_PUT, path);
}

/**
 * DELETEリクエストを送信します。
 * @param path リクエストパス
 * @return ステータスコード (取得に失敗した場合: -1)
 */
int LpwaV4HttpClient::del(const char *path)
{
  return sendRequest(HTTP_REQUEST_METHOD_DELETE, path);
}

/**
 * DELETEリクエストを送信します。
 * @param path リクエストパス
 * @param contentType Content-Type ヘッダー
 * @param body リクエストボディ
 * @return ステータスコード (取得に失敗した場合: -1)
 */
int LpwaV4HttpClient::del(const char *path, const char *contentType,
                          const char *body)
{
  _contentType = contentType;
  setBody(body);
  return sendRequest(HTTP_REQUEST_METHOD_DELETE, path);
}

/**
 * リクエストを送信します。
 * @param method HTTPメソッド ("GET", "POST", "PUT", "DELETE")
 * @param path リクエストパス
 * @return ステータスコード (取得に失敗した場合: -1)
 */
int LpwaV4HttpClient::sendRequest(HttpRequestMethodV4 method, const char *path)
{
  const unsigned long start = millis();
  _status = -1;
  _body = "";
  _contentLength = -1;
  char *resultp;
  char msg[32];
  int rcvlen = 0;
  int p1 = -1, p2 = -1, p3 = -1;

  int show_resp = 0 /* Whether to show HTTP headers */;

  /*
    theMurataLpwaCore.sendCmd("AT%HTTPCFG=\"ABORT\",1\r");
    if (theMurataLpwaCore.waitForResponse("OK\r",NULL,0,30000) < 0) {
      _close();
      return _status = -1;
    }
  */

  if (_https)
  {
    String uri = "https://" + _host + ":" + _port + path;
    theMurataLpwaCore.sendf("AT%%HTTPCFG=\"NODES\",1,\"%s\"\r", uri.c_str());
    if (theMurataLpwaCore.waitForResponse("OK\r", NULL, 0, 30000) < 0)
    {
      _close();
      return _status = -1;
    }
    /** Use TLS **/
    theMurataLpwaCore.sendCmd("AT%HTTPCFG=\"TLS\",1,2,10\r");
    if (theMurataLpwaCore.waitForResponse("OK\r", NULL, 0, 10000) < 0)
    {
      _close();
      return _status = -1;
    }
  }
  else
  {
    String uri = "http://" + _host + ":" + _port + path;
    theMurataLpwaCore.sendf("AT%%HTTPCFG=\"NODES\",1,\"%s\"\r", uri.c_str());
    if (theMurataLpwaCore.waitForResponse("OK\r", NULL, 0, 30000) < 0)
    {
      _close();
      return _status = -1;
    }
  }

  String strHeaders = "";
  if (_requestHeader.length() > 0)
  {
    for (int p = 0; p < _requestHeader.length();)
    {
      int next = _requestHeader.indexOf("\r\n", p);
      if (next < 0)
        break;
      strHeaders += ",\"";
      strHeaders += _requestHeader.substring(p, next);
      strHeaders += "\"";
      p = next + 2;
    }
  }

  /** Send Request **/
  String cmd = "";
  switch (method)
  {
  case HTTP_REQUEST_METHOD_GET:
    cmd = "AT%HTTPCMD=\"GET\",1";
    if (strHeaders.length() > 0)
    {
      cmd += ",,0,1";
      cmd += strHeaders;
    }
    cmd += "\r";
    theMurataLpwaCore.sendCmd(cmd.c_str());
    if (theMurataLpwaCore.waitForResponse("OK\r", NULL, 0, 10000) < 0)
    {
      _close();
      return _status = -1;
    }
    if (theMurataLpwaCore.waitForResponse("%HTTPEVU:\"GETRCV\"", _rcvBuffer, RCVBUFFSIZE, 30000) < 0)
    {
      _close();
      return _status = -1;
    }
    resultp = strchr(_rcvBuffer, ',');
    break;

  case HTTP_REQUEST_METHOD_POST:
  {
    int bodyLength = _requestBody.length();

    cmd = "AT%HTTPSEND=\"POST\",1,";
    cmd += bodyLength;
    cmd += ",,\"";
    cmd += _contentType;
    cmd += "\",0,0";
    if (strHeaders.length() > 0)
    {
      cmd += strHeaders;
    }
    cmd += "\r";
    cmd += _requestBody;
    theMurataLpwaCore.sendCmd(cmd.c_str());

    if (theMurataLpwaCore.waitForResponse("OK\r", NULL, 0, 10000) < 0)
    {
      _close();
      return _status = -1;
    }
    if (theMurataLpwaCore.waitForResponse("%HTTPEVU:\"POSTCONF\"", _rcvBuffer, RCVBUFFSIZE, 30000) < 0)
    {
      _close();
      return _status = -1;
    }
    // 2回 %HTTPEVU イベントが来る??
    //    if (theMurataLpwaCore.waitForResponse("%HTTPEVU:\"POSTCONF\"",_rcvBuffer,RCVBUFFSIZE,30000) < 0) {
    //      _close();
    //      return _status = -1;
    //    }
  }
    resultp = strchr(_rcvBuffer, ',');
    break;

  case HTTP_REQUEST_METHOD_PUT:
    cmd = "AT%HTTPSEND=\"PUT\",1,";
    cmd += _requestBody.length();
    cmd += ",,\"";
    cmd += _contentType;
    cmd += "\",0,0";
    if (strHeaders.length() > 0)
    {
      cmd += strHeaders;
    }
    cmd += "\r";
    cmd += _requestBody;
    theMurataLpwaCore.sendCmd(cmd.c_str());

    if (theMurataLpwaCore.waitForResponse("OK\r", NULL, 0, 10000) < 0)
    {
      _close();
      return _status = -1;
    }
    if (theMurataLpwaCore.waitForResponse("%HTTPEVU:\"PUTCONF\"", _rcvBuffer, RCVBUFFSIZE, 30000) < 0)
    {
      _close();
      return _status = -1;
    }
    // 2回 %HTTPEVU イベントが来る??
    if (theMurataLpwaCore.waitForResponse("%HTTPEVU:\"PUTCONF\"", _rcvBuffer, RCVBUFFSIZE, 30000) < 0)
    {
      _close();
      return _status = -1;
    }
    resultp = strchr(_rcvBuffer, ',');
    break;

  case HTTP_REQUEST_METHOD_DELETE:
    cmd = "AT%HTTPCMD=\"DELETE\",1";
    //    if(strHeaders.length() > 0) {
    //      cmd += ",,0,1";
    //      cmd += strHeaders;
    //    }
    cmd += "\r";
    theMurataLpwaCore.sendCmd(cmd.c_str());
    if (theMurataLpwaCore.waitForResponse("OK\r", NULL, 0, 10000) < 0)
    {
      _close();
      return _status = -1;
    }
    if (theMurataLpwaCore.waitForResponse("%HTTPEVU:\"DELCONF\"", _rcvBuffer, RCVBUFFSIZE, 30000) < 0)
    {
      _close();
      return _status = -1;
    }
    // 2回 %HTTPEVU イベントが来る??
    if (theMurataLpwaCore.waitForResponse("%HTTPEVU:\"DELCONF\"", _rcvBuffer, RCVBUFFSIZE, 30000) < 0)
    {
      _close();
      return _status = -1;
    }
    resultp = strchr(_rcvBuffer, ',');
    break;

  default:
    _close();
    return _status = -1;
  }
  sscanf(resultp, ",%d,%d,%d,%d", &p1, &p2, &p3, &_contentLength);
  theMurataLpwaCore.debugPrint(3, "Event %d,%d,%d,%d\r\n", p1, p2, p3, _contentLength);
  if (p2 != 0)
  {
    return _status = -1;
  }
  _body = "";
  _chunked = 0;
  char *resp;
  for (int loop = 0;; ++loop)
  {
    int szmax = (MAXHTTPBODYRCVSIZE - rcvlen);
    theMurataLpwaCore.debugPrint(3, "szmax:%d\r\n", szmax);
    if (szmax <= 0)
      break;
    delay(1000);
    theMurataLpwaCore.debugPrint(3, "loop: %d\r\n", loop);
    theMurataLpwaCore.sendf("AT%%HTTPREAD=1,%d\r", min(3000, szmax));
    if (theMurataLpwaCore.waitForResponse("%HTTPREAD:", _rcvBuffer, RCVBUFFSIZE, 60000) < 0)
    {
      _close();
      break;
    }
    char *dataszp = strstr(_rcvBuffer, "HTTPREAD:") + 9;
    int datasz = atoi(dataszp);
    int tsz = atoi(strchr(dataszp, ',') + 1);
    char *strResponse = theMurataLpwaCore.readBytes(datasz, _chunked);
    rcvlen += datasz;
    theMurataLpwaCore.debugPrint(3, "datasz: %d, %d\r\n", datasz, tsz);
    if (loop == 0)
    {
      char *bp = strstr(strResponse, "\r\n\r\n");
      *(bp + 2) = 0;
      _headerLength = bp - strResponse + 4;
      _chunked = strstr(strResponse, "Transfer-Encoding: chunked") ? 1 : 0;
      _status = atoi(strchr(strResponse, ' ') + 1);
      resp = bp + 4;
      theMurataLpwaCore.debugPrint(3, "chunked:%d\r\n", _chunked);
    }
    else
    {
      resp = strResponse;
    }
    _body += String(resp);
    theMurataLpwaCore.debugPrint(3, "size:(rcv)%d,(cont)%d,(head)%d\r\n", rcvlen, _contentLength, _headerLength);
    if (_contentLength >= 0)
    {
      if (rcvlen - _headerLength >= _contentLength)
        break;
    }
    else if (_chunked)
    {
      theMurataLpwaCore.debugPrint(3, "body:##%s##\r\n", _body.substring(_body.length() - 10).c_str());
      if (_body.endsWith("\r\n0\r\n\r\n"))
        break;
    }
    else
    {
      if (datasz == tsz)
        break;
    }
    delay(500);
  }

  if (_chunked)
  { // チャンクを接続する
    String b2 = "";
    char *p = (char *)_body.c_str();
    for (;;)
    {
      int sz = strtol(p, NULL, 16);
      p = strchr(p, '\n') + 1;
      if (p)
      {
        *(p + sz) = 0;
        b2 += String(p);
        p += (sz + 2);
        if (sz == 0)
          break;
      }
      else
        break;
    }
    _body = b2;
  }

  switch (method)
  {
  case HTTP_REQUEST_METHOD_GET:
  case HTTP_REQUEST_METHOD_POST:
  case HTTP_REQUEST_METHOD_PUT:
  case HTTP_REQUEST_METHOD_DELETE:
    theMurataLpwaCore.debugPrint(2, "@@@@@@ HTTP response code :%d\r\n", _status);
    theMurataLpwaCore.debugPrint(3, "@ Body [%s]\r\n", _body.c_str());
    for (int i = 10; i; --i)
    {
      int c = _body.charAt(_body.length() - i);
      theMurataLpwaCore.debugPrint(3, "[%02x]", c);
    }
    theMurataLpwaCore.debugPrint(3, "\r\n");
    break;
  default:
    _status = -1;
  }
  _close();
  return _status;
}

/**
 * レスポンスに含まれるHTTPステータスコードを取得します。
 * @return HTTPステータスコード (取得に失敗した場合: -1)
 */
int LpwaV4HttpClient::responseStatusCode()
{
  return _status;
}

/**
 * レスポンスボディを取得します。
 * @return レスポンスボディ (取得に失敗した場合: "")
 */
String LpwaV4HttpClient::responseBody()
{
  return _body;
}

/**
 * HTTPリクエストヘッダーを削除します。
 * @param なし
 */
void LpwaV4HttpClient::clrHeader()
{
  _requestHeader = "";
}

/**
 * HTTPリクエストヘッダーを追加します。
 * @param name ヘッダーの名前
 * @param value ヘッダーの値
 */

/*template <class T, class U> void LpwaV4HttpClient::addHeader(T name, U value) {
  _requestHeader += name;
  _requestHeader += ':';
  _requestHeader += value;
  _requestHeader += "\r\n";
}*/

void LpwaV4HttpClient::addHeader(String name, String value)
{
  _requestHeader += name;
  _requestHeader += ':';
  _requestHeader += value;
  _requestHeader += "\r\n";
}

/**
 * HTTPリクエストボディを指定します。
 * @param body リクエストボディ
 */
template <class T>
void LpwaV4HttpClient::setBody(T body)
{
  _requestBody = body;
}

/**
 * HTTPS証明書を登録します
 * @param cert .pem形式の証明書データ、またはプリインストールされている証明書のファイル名
 * プリインストールされている証明書は下の通り:
 *   "Motive_crt3.pem",   "AmazonRootCA1.pem",      "BaltimoreRootCA.pem",    "kyo.ku",
 *   "sb.pem",            "AmazonRootCA3.pem",      "globalsign_root_r3.pem", "VeriSignPrimaryCA.pem",
 *   "AmazonRootCA4.pem", "Motive_crt1.pem",        "AmazonRootCA2.pem",      "DigiCert.pem",
 *   "verisign.pem",      "Motive_crt2.pem"
 * @return ステータスコード (取得に失敗した場合: -1)
 */
int LpwaV4HttpClient::setCert(const char *cert)
{
  theMurataLpwaCore.debugPrint(2, "@@@@@ setCert()enter\r\n");
  char *dir = "\".\"";
  char *pem = "prof1.pem";
  String cmd;
  if (strstr(cert, "-----") != NULL)
  { // 証明書をインストールする場合
    cmd = "AT%CERTCMD=\"WRITE\",\"prof1.pem\",0,\r\"";
    cmd += cert;
    cmd += "\"";
    theMurataLpwaCore.sendCmd(cmd.c_str());
    if (theMurataLpwaCore.waitForResponse("OK\r", NULL, 0, 10000) < 0)
    {
      return -1;
    }
  }
  else
  { // プリインストールされている証明書を使用する場合
    pem = (char *)cert;
    dir = "\"~\"";
  }
  cmd = "AT%CERTCFG=\"ADD\",10,\"";
  cmd += pem;
  cmd += "\",";
  cmd += dir;
  cmd += "\r";
  theMurataLpwaCore.sendCmd(cmd.c_str());
  if (theMurataLpwaCore.waitForResponse("OK\r", NULL, 0, 10000) < 0)
  {
    return -1;
  }
  return 0;
}
