package com.furture.litebrowser.common;

import android.graphics.DashPathEffect;
import android.graphics.PathEffect;
import android.support.annotation.Nullable;

/**
 * Created by furture on 2018/3/13.
 */

public class BorderStyles {

    public static final  int SOLID = 4;
    public static final  int DASHED = 3;
    public static final  int DOTTED = 2;


    public static @Nullable PathEffect getPathEffect(int borderStyle, int borderWidth) {
        switch (borderStyle) {
            case SOLID:
                return null;
            case DASHED:
                return new DashPathEffect(
                        new float[] {borderWidth*3, borderWidth*3, borderWidth*3, borderWidth*3}, 0);
            case DOTTED:
                return new DashPathEffect(
                        new float[] {borderWidth, borderWidth, borderWidth, borderWidth}, 0);
            default:
                return null;
        }
    }
}
