/**
 * @description 作为入口文件，主要负责引入 App (components container)、model，同时初始化 dva 等。
 * 入口文件应当保持尽可能的简洁。
 * @author amas
 */

// 引入各类基础依赖
import React from 'react';
import dva from 'dva/mobile';
import App from './App';
import Lj from '../../common/lengjing.middleware';

// Base 引入
import '../../common/base';

// Model引入
import CountModel from './models/model';

// 初始化dva
const app = dva();

// 注入数据
app.model(CountModel);

// Router
app.router(() => <App />);

app.use({
  onAction: Lj,
});

app.router(()=> <App />);

// 页面渲染
app.start(document.getElementById('root'));
