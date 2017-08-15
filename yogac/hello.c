#include "yoga/Yoga.h"
#include <time.h>

static double now_ms(void) {
    struct timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    return 1000.0 * res.tv_sec + (double) res.tv_nsec / 1e6;
}

int main(int argc, char const *argv[]) {
  double start = now_ms();
  YGNodeRef root = YGNodeNew();
  YGNodeStyleSetWidth(root, 200);
  YGNodeStyleSetFlexDirection(root, YGFlexDirectionColumn);
  YGNodeStyleSetAlignItems(root, YGAlignCenter);
  for(int i=0; i<5000; i++){
      YGNodeRef child = YGNodeNew();
      YGNodeStyleSetWidth(child, 20*i);
      YGNodeStyleSetHeight(child, 10*i);
      YGNodeInsertChild(root, child, i);
  }

  YGNodeCalculateLayout(root, 200, 200, YGDirectionLTR);

  printf("used %f \n", (now_ms() - start));


  YGNodeFreeRecursive(root);
  /* code */
  return 0;
}
