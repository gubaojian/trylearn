package com.furture.nestscroll;

import android.graphics.Color;
import android.support.design.widget.AppBarLayout;
import android.support.design.widget.CoordinatorLayout;
import android.support.design.widget.HeaderScrollBehavior;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        View view = findViewById(R.id.appbar_layout);

        CoordinatorLayout.LayoutParams params = (CoordinatorLayout.LayoutParams) view.getLayoutParams();
        HeaderScrollBehavior headerScrollBehavior = new HeaderScrollBehavior();
        headerScrollBehavior.setHeadOffset(200);
        params.setBehavior( headerScrollBehavior);

        final RecyclerView recyclerView = (RecyclerView) findViewById(R.id.recycle_view);

        final View lifeView = findViewById(R.id.life);

        findViewById(R.id.header).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
               // recyclerView.getAdapter().notifyDataSetChanged();
                //if(lifeView.getParent() != null){
                //    ((ViewGroup)lifeView.getParent()).removeView(lifeView);
               // }else{
                //    ((ViewGroup)findViewById(R.id.header).getParent()).addView(lifeView);
               // }
                if(lifeView.getVisibility() != View.VISIBLE){
                    lifeView.setVisibility(View.VISIBLE);
                }else{
                    lifeView.setVisibility(View.INVISIBLE);
                }


            }
        });
        final SwipeRefreshLayout refreshLayout = (SwipeRefreshLayout) findViewById(R.id.swipe_layout);
        refreshLayout.setOnRefreshListener(new SwipeRefreshLayout.OnRefreshListener() {
            @Override
            public void onRefresh() {
                refreshLayout.postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        refreshLayout.setRefreshing(false);
                    }
                }, 2000);
            }
        });

        View container = findViewById(R.id.container);
        CoordinatorLayout.LayoutParams containerParams = (CoordinatorLayout.LayoutParams) container.getLayoutParams();
        containerParams.setBehavior(new AppBarLayout.ScrollingViewBehavior());

        recyclerView.setLayoutManager(new LinearLayoutManager(getBaseContext()));
        recyclerView.setAdapter(new RecyclerView.Adapter<TextViewHolder>(){

            @Override
            public TextViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
                View itemView = LayoutInflater.from(getBaseContext()).inflate(R.layout.recycler_item, null);
                return new TextViewHolder( itemView);
            }

            @Override
            public void onBindViewHolder(TextViewHolder holder, int position) {
                  TextView itemView = (TextView) holder.itemView.findViewById(R.id.textview);
                  itemView.setText("RecycleViewList Item " + position);
            }



            @Override
            public int getItemCount() {
                return 100;
            }
        });
        recyclerView.setBackgroundColor(Color.RED);
        recyclerView.getAdapter().notifyDataSetChanged();
    }
}
