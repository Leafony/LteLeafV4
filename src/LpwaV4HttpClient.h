#ifndef LPWA_V4_HTTP_CLIENT_H_
#define LPWA_V4_HTTP_CLIENT_H_

#include "MurataLpwaCore.h"
#include "typesV4.h"
#include <Arduino.h>

#define MAXHTTPBODYRCVSIZE 20000
#define RCVBUFFSIZE 3200

//extern void debugPrint(String s);
//extern void debugPrint(char* s);

/**
 * HTTPクライアントのためのクラス
 */
class LpwaV4HttpClient {
  enum HttpRequestMethodV4 {
    HTTP_REQUEST_METHOD_GET,
    HTTP_REQUEST_METHOD_POST,
    HTTP_REQUEST_METHOD_PUT,
    HTTP_REQUEST_METHOD_DELETE
  };
  String _host;
  uint16_t _port;
  bool _https;
  int _status;
  int _contentLength;
  int _headerLength;
  int _chunked;
  String _body;
  char _rcvBuffer[RCVBUFFSIZE];
  String _requestHeader;
  String _requestBody;
  String _contentType;
  void _close();

public:
  LpwaV4HttpClient(const char *host, uint16_t port, const bool https = false);
  LpwaV4HttpClient(String &host, uint16_t port, const bool https = false);
  ~LpwaV4HttpClient();
  int get(const char *path);
  int post(const char *path, String &contentType, String &body) {
    return post(path, contentType.c_str(), body.c_str());
  }
  int post(const char *path, const char *contentType, const char *body);
  int put(const char *path, String &contentType, String &body) {
    return put(path, contentType.c_str(), body.c_str());
  }
  int put(const char *path, const char *contentType, const char *body);
  int del(const char *path);
  int del(const char *path, String &contentType, String &body) {
    return del(path, contentType.c_str(), body.c_str());
  }
  int del(const char *path, const char *contentType, const char *body);
  int sendRequest(HttpRequestMethodV4 method, const char *path);
  int responseStatusCode();
  String responseBody();
  void clrHeader();
//  template <class T, class U> void addHeader(T name, U value);
  void addHeader(String name, String value);
  void addHeader(const char *name, const char *value) {
    addHeader(String(name), String(value));
  }
  template <class T> void setBody(T body);

  int setCert(const char *cert);
};

#endif
