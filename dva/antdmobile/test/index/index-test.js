/**
 * @file index-test
 * @author anima
 */

import {expect, assert} from 'chai';
import React from 'react';
import dva from 'dva/mobile';
import {call, put} from 'dva/effects';
import CountModel, {delay} from '../../src/pages/index/models/model';

describe('index', () => {
  const app = dva();

  app.model(CountModel);
  app.router(() => <div />);
  app.start();

  it('init value', () => {
    expect(app._store.getState().amas.current).to.equal(0);
  });

  it('reducer -- count/add', () => {

    app._store.dispatch({type: 'count/add'});

    expect(app._store.getState().amas.current).to.equal(1);

  });

  it('reducer -- count/minus', () => {
    app._store.dispatch({type: 'count/minus'});

    expect(app._store.getState().amas.current).to.equal(0);
  });

  it('effects -- count/add', () => {
    const generator = CountModel.effects['count/add']();

    assert.deepEqual(generator.next().value, call(delay, 1000));

    assert.deepEqual(generator.next().value, put({type: 'count/minus'}));

    assert.deepEqual(generator.next(), {done: true, value: undefined});
  });
});
