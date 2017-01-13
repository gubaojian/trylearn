const lengjingPlugin = require('@alipay/lengjing-webpack-plugin');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const path = require('path');
const Visualizer = require('webpack-visualizer-plugin');
const webpack = require('atool-build/lib/webpack');
const px2rem = require('postcss-plugin-px2rem');
const lengjingConfig = require('../lengjing.config');

const commonConfig = function(webpackConfig) {

  const isMock = process.env.NODE_ENV === 'mock';
  const isDev = process.env.NODE_ENV === 'development';

  /**
   * Css Module 处理逻辑,忽略node_modules里面的样式文件
   */
  webpackConfig.module.loaders.forEach(function(loader) {
    if (typeof loader.test === 'function' && loader.test.toString().indexOf('\\.less$') > -1) {
      loader.include = /node_modules/;
      loader.test = /\.less$/;
    }
    if (loader.test.toString() === '/\\.module\\.less$/') {
      loader.exclude = /node_modules/;
      loader.test = /\.less$/;
    }
    if (typeof loader.test === 'function' && loader.test.toString().indexOf('\\.css$') > -1) {
      loader.include = /node_modules/;
      loader.test = /\.css$/;
    }
    if (loader.test.toString() === '/\\.module\\.css$/') {
      loader.exclude = /node_modules/;
      loader.test = /\.css$/;
    }
  });

  /**
   * 增加transform-runtime
   */
  webpackConfig.babel.plugins.push('transform-runtime');

  /**
   * html文件处理逻辑
   */
  webpackConfig.entry = lengjingConfig.pages.reduce((entryMap, metaData) => {
    const entry = metaData.entry.substring(0, metaData.entry.length - 5);

    webpackConfig.plugins.push(new HtmlWebpackPlugin({
      inject: false,
      minify: {
        collapseWhitespace: true,
      },
      filename: metaData.entry,
      antBridgeVersion: '1.1.1',
      env: process.env.NODE_ENV,
      title: metaData.title,
      entry: entry,
      template: path.join(__dirname, '../src/common/entry.ejs'),
    }));

    entryMap[entry] = metaData.src;
    return entryMap;
  }, {});
  // NOTE 不用生成模板 js 只能手写
  webpackConfig.entry.hd = './src/common/hd.js';

  /**
   * 添加棱镜处理逻辑
   */
  if (process.env.NODE_ENV === 'lengjing') {
    webpackConfig.plugins.push(
      new lengjingPlugin.Mock({})
    );
  }

  /**
   * 用来分析项目依赖文件体积
   */
  webpackConfig.plugins.push(new Visualizer());

  /**
   * 公共文件抽取逻辑
   */
  webpackConfig.plugins.some(function(plugin, i) {
    if (plugin instanceof webpack.optimize.CommonsChunkPlugin) {
      webpackConfig.plugins.splice(i, 1, new webpack.optimize.CommonsChunkPlugin({
        name: 'common',
        minChunks: 2,
      }));

      return true;
    }
  });


  /**
   * Production Environment, reference to an external resource of React / ReactDOM
   */
  if (process.env.NODE_ENV === 'production') {
    webpackConfig.externals = {
      'react': 'React',
      'react-dom': 'ReactDOM',
    };
  }

  /**
   * 开发环节添加source-map
   */
  if (isMock || isDev) {
    webpackConfig.devtool = 'source-map';
  }

  /**
   * 高清方案
   */
  const px2remOpts = {
    rootValue: 100,
  };
  webpackConfig.postcss.push(px2rem(px2remOpts));
};

module.exports = commonConfig;

