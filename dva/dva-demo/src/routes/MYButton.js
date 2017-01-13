import {connect} from 'dva'
import MYButton from '../components/MYButton'

const mapStateToProps = function(state){
    console.log(state);
    return {
      text : state.MYButton.value,
      data : state.MYButton.data,
    };
};

const mapDispatchToProps = (dispatch)=>{
  return {
    onClick : ()=>{
      dispatch({type:'MYButton/click'})
    }
  };
};

const MYButtonContainer = connect(mapStateToProps, mapDispatchToProps)(MYButton);

export default MYButtonContainer;
