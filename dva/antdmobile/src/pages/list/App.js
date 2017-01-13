/**
 * @description App 是一个 components 的 container 文件，主要用来承载所有跟 UI 界面相关的 components
 * @author amas
 */

import React from 'react';
import {connect} from 'dva/mobile';
import { Tabs, WhiteSpace,ActivityIndicator,List} from 'antd-mobile';
import classnames from 'classnames';
import styles from './App.less';

import Button from './components/button';
import Panel from './components/panel'
import Text from '../../components/text';

const StatusConfig = {
   'ALL' : {
     title :'全部资产(元)',
     desc : '全部',
   },
   'SELL' : {
     title :'卖出资产(元)',
     desc : '卖出',
   },
   'BUY' : {
     title :'买入资产(元)',
     desc : '买入',
   },
   'REWARD' : {
     title : '收益资产(元)',
     desc : '收益',
   }
}

class ListTitleSubTitle extends React.Component {
  render(){
    var props = this.props;
    return (<div>
         {props.title}
        <List.Item.Brief>
         {props.subTitle}
        </List.Item.Brief>
    </div>);
  }
}

class Count extends React.Component {



  render() {
    var rst = this.props.rst;
    if(!rst || !rst.data || !rst.data.records){
      return (<div className={styles.normal}></div>);
    }
    var records = rst.data.records || [];
    var amount = rst.data.total;
    var status = rst.status;
    var title = '全部资产(元)';
    if(status == 'ALL'){
      amount = rst.data.total;
      title = '全部资产(元)';
    }else if(status == 'SELL'){
      amount = rst.data.totalSell;
      title = '卖出资产(元)';
    }else if(status == 'BUY'){
      amount = rst.data.totalBuy;
      title = '买入资产(元)';
    }else if(status == 'REWARD'){
      amount = rst.data.totalReward
      title = '收益资产(元)';
    }
    records =   records.filter(record=>{
      if(status == 'ALL'){
        return true;
      }
       if(record.orderStatus == status){
         return true;
       }
       return false;
    });

    return (
      <div className={styles.normal}>
      <Tabs defaultActiveKey="1" animated={false} onChange={this.props.tabChanged}>
         <Tabs.TabPane tab="全部" key="ALL">
         </Tabs.TabPane>
         <Tabs.TabPane tab="买入" key="BUY">
         </Tabs.TabPane>
         <Tabs.TabPane tab="卖出" key="SELL">
         </Tabs.TabPane>
         <Tabs.TabPane tab="收益" key="REWARD">
         </Tabs.TabPane>
         </Tabs>
         <div style={{alignItems: 'center', justifyContent: 'center'}}>
            <Panel amount={amount} title={title}/>
            <WhiteSpace size="xl" />
            <List>
               {
                 records.map((record, index)=>{
                   return (
                     <List.Item key={index} arrow="horizontal" extra={<ListTitleSubTitle title={record.applyAmount.cent} subTitle={StatusConfig[record.orderStatus].desc}/>}>
                          <ListTitleSubTitle title={record.productName} subTitle="2016/12/6 下午11:49:20"/>
                     </List.Item>
                   )
                 })
               }
            </List>
         </div>
      </div>
    );
  }
}

function mapStateToProps(state) {
  return {rst: state.amas};
}

function mapDispatchToProps(dispatch) {
  dispatch({
    type : 'amas/initListData'
  });
  return {
    tabChanged(key) {
      dispatch({
        type : 'amas/tabChanged',
        playload : key,
      })
    },
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
