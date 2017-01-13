const jsAlias = function(webpackConfig, map) {
  webpackConfig.babel = webpackConfig.babel || {};
  webpackConfig.babel.plugins = webpackConfig.babel.plugins || [];
  webpackConfig.babel.plugins.push(['module-alias', map]);
};

module.exports = jsAlias;
