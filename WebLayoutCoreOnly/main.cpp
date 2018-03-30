#include <iostream>
#include <wtf/Platform.h>
#include <wtf/Function.h>
#include <wtf/text/WTFString.h>
#include <wtf/Threading.h>
#include "Source/WebCore/RenderStyle.h"
#include "Source/WebCore/IntSize.h"
#include "Source/WebCore/RenderObject.h"

using namespace WebCore;

int main() {
    WTF::initializeMainThread();
    AtomicString::init();
    printf("Current Thread %d " , currentThread());
    WTF::String string("love");
    WebCore::RenderStyle renderStyle = WebCore::RenderStyle::create();
    IntSize size(10, 10);
    renderStyle.setBorderRadius(size);
    renderStyle.setDisplay(FLEX);
    renderStyle.setWidth(Length(10, Auto));
    renderStyle.setHeight(Length(100, Percent));
    renderStyle.border().top();
    renderStyle.setMarginLeft(Length(10, Auto));
    renderStyle.setPaddingBottom(Length(10, Auto));
    renderStyle.setBorderTopWidth(10);
    renderStyle.setFloating(LeftFloat);
    renderStyle.setFlexWrap(FlexNoWrap);
    renderStyle.setOverflowWrap(BreakOverflowWrap);
    renderStyle.setMaxHeight(Length(10, Auto));
    renderStyle.setDirection(LTR);
    //renderStyle.fontMetrics();
    //renderStyle.setContent()

    std::cout <<  "is null " << (renderStyle.hasBorder()) << std::endl;
    std::cout << "Hello, World!" << renderStyle.width().intValue()  << "top" <<renderStyle.border().top().width() << std::endl;
    return 0;
}