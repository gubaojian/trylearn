/**
 * @file getJSON
 */

import {factory} from './factory';

const getJSON = factory();

/**
 * 获取数据的方法
 *
 * @param {String} url 请求接口，类似ajax的url，如 alipay.pcredit.huabei.prod.main
 * @param {Object} param 请求参数
 * @param {Function} callback 回调函数
 * @return {void}
 */

const getData = (url, param, callback) => {
  const data = getJSON(url, param);
  // 添加延迟，逼真请求
  setTimeout(() => {
    callback(data);
  }, 100);
};

export default getData;
