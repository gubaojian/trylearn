import React from 'react';
import { Router, Route } from 'dva/router';
import IndexPage from './routes/IndexPage';
import MYButton from './routes/MYButton';

function RouterConfig({ history }) {
  return (
    <Router history={history}>
       <Route path="/" component={IndexPage} />
       <Route path="/button" component={MYButton}/>
    </Router>
  );
}

export default RouterConfig;
