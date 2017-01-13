const RewriteImportPlugin = require('less-plugin-rewrite-import');

const lessAlias = function(webpackConfig, map) {
  webpackConfig.lessLoader = webpackConfig.lessLoader || {};
  webpackConfig.lessLoader.lessPlugins = webpackConfig.lessLoader.lessPlugins || [];
  webpackConfig.lessLoader.lessPlugins.push(
    new RewriteImportPlugin({
      paths: map,
    })
  );
};

module.exports = lessAlias;
