package android.support.design.widget;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;

import java.lang.reflect.Method;

/**
 * Created by furture on 2017/10/30.
 */

public class HeaderScrollBehavior  extends AppBarLayout.Behavior  {

    private int mHeadOffset;

    private boolean mSkipNestedPreScroll;

    private static final String TAG = "HeadOffsetBehavior";

    public HeaderScrollBehavior() {
    }

    public HeaderScrollBehavior(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public void setHeadOffset(int offset) {
        mHeadOffset = offset;
    }


    // ---------------- offset处理: 用户在Head上scroll/fling ----------------------------


    @Override
    int getMaxDragOffset(AppBarLayout view) {
        /*复写了父类非公开方法，有一定风险*/
        if(mHeadOffset <= 0 || mHeadOffset > view.getMeasuredHeight()) {
            return super.getMaxDragOffset(view);
        } else {
            return -getNormalizedOffset(mHeadOffset,view.getMeasuredHeight());
        }

    }

    @Override
    int getScrollRangeForDragFling(AppBarLayout view) {
        /*复写了父类非公开方法，有一定风险*/
        if(mHeadOffset <= 0 || mHeadOffset > view.getMeasuredHeight()) {
            return super.getScrollRangeForDragFling(view);
        } else {
            return getNormalizedOffset(mHeadOffset,view.getMeasuredHeight());
        }
    }


    // ---------------- offset处理: 用户在滚动容器上scroll/fling -------------------------

    // 先全部走hack的behavior，不走原生的

    /**
     * 此方法是support 26.x新加的，必须复写，
     * */
    @SuppressWarnings("unused")
    public void onNestedPreScroll(CoordinatorLayout coordinatorLayout, AppBarLayout child,
                                  View target, int dx, int dy, int[] consumed, int type) {
        if(type == 0/*ViewCompat.TYPE_TOUCH*/) {
            this.onNestedPreScroll(coordinatorLayout,child,target,dx,dy,consumed);
        }
    }

    @Override
    public void onNestedPreScroll(CoordinatorLayout coordinatorLayout, AppBarLayout child,
                                  View target, int dx, int dy, int[] consumed) {
        if (dy != 0 && !mSkipNestedPreScroll) {
            int min, max;
            if (dy < 0) {
                // We're scrolling down
                min = -child.getTotalScrollRange();
                max = min + child.getDownNestedPreScrollRange();

            } else {
                // We're scrolling up
                if(mHeadOffset <= 0 || mHeadOffset > child.getMeasuredHeight()) {
                    min = -child.getUpNestedPreScrollRange();
                } else {
                    min = -getNormalizedOffset(mHeadOffset, child.getMeasuredHeight());
                }

                max = 0;
            }
            consumed[1] = scroll(coordinatorLayout, child, dy, min, max);
        }
    }

    @Override
    public void onNestedScroll(CoordinatorLayout coordinatorLayout, AppBarLayout child,
                               View target, int dxConsumed, int dyConsumed,
                               int dxUnconsumed, int dyUnconsumed) {
        if (dyUnconsumed < 0) {
            // If the scrolling view is scrolling down but not consuming, it's probably be at
            // the top of it's content
            scroll(coordinatorLayout, child, dyUnconsumed,
                    -child.getDownNestedScrollRange(), 0);
            // Set the expanding flag so that onNestedPreScroll doesn't handle any events
            mSkipNestedPreScroll = true;
        } else {
            // As we're no longer handling nested scrolls, reset the skip flag
            mSkipNestedPreScroll = false;
        }
    }

    @Override
    public void onStopNestedScroll(CoordinatorLayout coordinatorLayout, AppBarLayout abl,
                                   View target) {
        /*这里先移除了snap特性*/
        // Reset the flags
        mSkipNestedPreScroll = false;
    }

    @Override
    public boolean onNestedFling(final CoordinatorLayout coordinatorLayout,
                                 final AppBarLayout child, View target, float velocityX, float velocityY,
                                 boolean consumed) {
        if (target instanceof RecyclerView) {
            final RecyclerView recyclerView = (RecyclerView) target;
            consumed = velocityY > 0 || recyclerView.computeVerticalScrollOffset() > 0;
        }
        boolean flung = false;

        if (!consumed) {
            // It has been consumed so let's fling ourselves
            flung = fling(coordinatorLayout, child, -child.getTotalScrollRange(),
                    0, -velocityY);
        } else {
            // If we're scrolling up and the child also consumed the fling. We'll fake scroll
            // up to our 'collapsed' offset
            if (velocityY < 0) {
                // We're scrolling down
                final int targetScroll = -child.getTotalScrollRange()
                        + child.getDownNestedPreScrollRange();
                if (getTopBottomOffsetForScrollingSibling() < targetScroll) {
                    // If we're currently not expanded more than the target scroll, we'll
                    // animate a fling
                    requestAnimateOffsetTo(coordinatorLayout, child, targetScroll, velocityY);
                    flung = true;
                }
            } else {
                // We're scrolling up
                int targetScroll;
                if(mHeadOffset <= 0 || mHeadOffset > child.getMeasuredHeight()) {
                    targetScroll = -child.getUpNestedPreScrollRange();
                } else {
                    targetScroll = -getNormalizedOffset(mHeadOffset,child.getMeasuredHeight());
                }
                if (getTopBottomOffsetForScrollingSibling() > targetScroll) {
                    // If we're currently not expanded less than the target scroll, we'll
                    // animate a fling
                    requestAnimateOffsetTo(coordinatorLayout, child, targetScroll, velocityY);
                    flung = true;
                }
            }
        }
        return flung;
    }


    private void requestAnimateOffsetTo(final CoordinatorLayout coordinatorLayout,
                                        final AppBarLayout child, final int offset, float velocity) {
        Class<?> clazz = AppBarLayout.Behavior.class;
        try {
            Method targetMethod = clazz.getDeclaredMethod("animateOffsetTo",CoordinatorLayout.class, AppBarLayout.class, int.class, float.class);
            targetMethod.setAccessible(true);
            targetMethod.invoke(this, coordinatorLayout, child, offset, velocity);
        }catch (Exception e) {
            Log.e(TAG, "error! can not found [animateOffsetTo] method");
        }
    }

    /**与iOS兼容*/
    private int getNormalizedOffset(int offset, int headHeight) {
        return headHeight - offset;
    }


}
