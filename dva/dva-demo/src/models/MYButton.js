import $ from 'jquery'

export default {
  namespace : 'MYButton',
  state : {
     value : 'Models Values',
     data : null,
  },
  reducers : {
    click(state, action){
      return {
        value : '现在是' + (new Date()).getFullYear() + '年'
      };
    }
  },
  effects : {
     click : *(action, {call, put})=>{
       yield put({type:'begin'});
       const url = 'https://os.alipayobjects.com/rmsportal/nnUJCKmLCyQHCex.json';
       const data = yield $.getJSON(url).then(data=>data);
       yield put({
         type : 'end',
         playload:data
       })
     }
  }
}
