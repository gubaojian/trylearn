/**
 * @description App 是一个 components 的 container 文件，主要用来承载所有跟 UI 界面相关的 components
 * @author amas
 */

import React from 'react';
import {connect} from 'dva/mobile';

import classnames from 'classnames';

import styles from './App.less';

import Button from './components/button';
import Text from '../../components/text';

class Count extends React.Component {
  render() {
    const {state, buttonClick} = this.props;
    const status = state.status;

    const textClass = classnames({
      [styles.current]: true,
      [styles.black]: status === 'initial',
      [styles.gray]: status === 'changing',
      [styles.red]: status === 'changed',
    });

    let btnTxt;
    switch (status) {
      case 'initial':
        btnTxt = 'Change Color';
        break;
      case 'changing':
        btnTxt = 'Changing';
        break;
      case 'changed':
        btnTxt = 'Reset Color';
        break;
    }

    return (
      <div className={styles.normal}>
        <div className={textClass}>
          <Text>Hello React</Text>
        </div>
        <div className={styles.button}>
          <Button
            type="primary"
            loading={status === 'changing'}
            onClick={ () => {
              if (status === 'changing') return;
              buttonClick(state.status === 'changed' ? 'amas/reset' : 'amas/change');
            }}
          >{ btnTxt }</Button>
        </div>
        <p>（点击按钮，尝试改变文本颜色）</p>
      </div>
    );
  }
}

function mapStateToProps(state) {
  return {state: state.amas};
}

function mapDispatchToProps(dispatch) {
  return {
    buttonClick: (actionName) => {
      dispatch(
        {
          type: actionName,
          lengjing: {
            seedName: 'antd-mobile',
            extra: {
              from: 'antd-mobile boilerplate',
            },
          },
        }
      );
    },
  };
}

const App = connect(mapStateToProps, mapDispatchToProps)(Count);

export default App;
