'use strict';

const commonConfig  = require('./scripts/common_config');
const alipayConfig  = require('./scripts/alipay_config');

module.exports = function(webpackConfig) {

  const runEnv = process.env.RUN_ENV || 'alipay';

  // 通用处理逻辑
  commonConfig(webpackConfig);

  // 渠道处理逻辑
  if (runEnv === 'alipay') {
    alipayConfig(webpackConfig);
  }

  // 返回 webpack 配置对象
  return webpackConfig;
};
