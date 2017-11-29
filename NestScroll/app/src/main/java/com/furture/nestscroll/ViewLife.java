package com.furture.nestscroll;

import android.content.Context;
import android.support.annotation.IntDef;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;

/**
 * Created by furture on 2017/11/1.
 */

public class ViewLife extends View {
    public ViewLife(Context context) {
        super(context);
    }

    public ViewLife(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
    }

    public ViewLife(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    public ViewLife(Context context, @Nullable AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
    }

    @Override
    protected void dispatchVisibilityChanged(@NonNull View changedView, int visibility) {
        Log.e("weex", "dispatchVisibilityChanged " + changedView);
        super.dispatchVisibilityChanged(changedView, visibility);
    }

    @Override
    protected void onVisibilityChanged(@NonNull View changedView,  int visibility) {
        Log.e("weex", "onVisibilityChanged " + changedView  +  visibility);
        super.onVisibilityChanged(changedView, visibility);
    }

    @Override
    public void dispatchWindowVisibilityChanged(int visibility) {
        Log.e("weex", "dispatchWindowVisibilityChanged " + visibility);
        super.dispatchWindowVisibilityChanged(visibility);
    }

    @Override
    protected void onWindowVisibilityChanged(int visibility) {
        Log.e("weex", "onWindowVisibilityChanged " + visibility);
        super.onWindowVisibilityChanged(visibility);
    }
}
