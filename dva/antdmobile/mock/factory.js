/**
 * @file mockFactory
 */

import Tracker from '@alipay/mtracker/lib/mtracker-ap';
const hashQuery = Tracker.Router.query || {};

/**
 * 生成mock方法
 *
 * @return {Function} mock文件中的Function
 */
export function factory() {
  /**
   * 请求入口
   *
   * @param {String} 请求路径
   * @param {Object=} 请求参数
   */
  return (path, param) => {
    const mockKey = './mockData/' + path;  //
    const viewId = hashQuery.mPageState || 'default';
    let dataFun;
    // 尝试直接去拿数据文件,如果拿不到就按[目录/场景]的格式去拿
    try {
      dataFun = require(mockKey)(viewId, param);
    } catch (err) {
      if (!dataFun) {
        try {
          dataFun = require(mockKey + '/' + viewId);
        }
        catch (err) {
          if (!dataFun) {
            dataFun = require(mockKey + '/' + 'default');
          }
        }
      }
    }
    const result = typeof dataFun === 'function' ? dataFun(param) : dataFun;
    if (result && result.hasOwnProperty('success')) {
      return result;
    }
    return {success: true, result: result};
  };
}
