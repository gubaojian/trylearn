/**
  * 获取数据的方法
  *
  * @param {string} url 请求接口，类似ajax的url，如 alipay.pcredit.huabei.prod.main
  * @param {Object} param 请求参数
  * @param {Function} callback 回调函数
  * @return {void}
  */
const getData = function(url, param, callback) {
  Ali.rpc({
    operationType: url,
    requestData: Array.isArray(param) ? param : [param],
    disableLimitView: true,
    headers: {
      appVersion: APP_VERSION, // APP_VERSION 是在 webpack 里配置的 alias，实现从package.json里面把应用版本号传递过来
      h5appid: APP_ID, // APP_ID 是在 webpack 里配置的 alias，实现从package.json里面把应用版本号传递过来
    },
  }, (result) => {
    DEVLOG;
    callback(result);
  });
};

export default getData;
