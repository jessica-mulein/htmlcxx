#include "stdafx.h"

using namespace htmlcxx2::HTML;

void test0()
{
    std::string html;
    ParserDom parser;
    Tree root = parser.parseTree(html);
    assert(root.begin() == root.end());

    html = "<div/>";
    root = parser.parseTree(html);
    assert(root.begin() != root.end());
}

void test1()
{
    std::string html(
R"(<div>
    Text
</div>between_text
<p>
    Text2<br>
</p>)");

    ParserDom parser;
    Tree root = parser.parseTree(html);
    Tree::pre_order_iterator deepIt = root.begin();
    assert(deepIt->isRoot());
    assert(deepIt->tagName()     == "");
    assert(deepIt->text()        == "");
    assert(deepIt->closingText() == "");
    assert(deepIt->content(html) == html);
    
    Tree::sibling_iterator siblingIt = deepIt.begin();
    assert(siblingIt->isTag());
    assert(siblingIt->tagName()     == "div");
    assert(siblingIt->text()        == "<div>");
    assert(siblingIt->closingText() == "</div>");
    assert(siblingIt->content(html) == "\n    Text\n");
    // or
    ++deepIt;
    assert(deepIt->isTag());
    assert(deepIt->tagName()     == "div");
    assert(deepIt->text()        == "<div>");
    assert(deepIt->closingText() == "</div>");
    assert(deepIt->content(html) == "\n    Text\n");
    
    ++siblingIt;
    assert(siblingIt->isText());
    assert(siblingIt->tagName()     == "");
    assert(siblingIt->text()        == "between_text\n");
    assert(siblingIt->closingText() == "");

    ++siblingIt;
    assert(siblingIt->isTag());
    assert(siblingIt->tagName()     == "p");
    assert(siblingIt->text()        == "<p>");
    assert(siblingIt->closingText() == "</p>");
    assert(siblingIt->content(html) == "\n    Text2<br>\n");

    ++deepIt;
    assert(deepIt->isText());
    assert(deepIt->tagName()     == "");
    assert(deepIt->text()        == "\n    Text\n");
    assert(deepIt->closingText() == "");

    deepIt = siblingIt.begin();
    assert(deepIt->isText());
    assert(deepIt->tagName()     == "");
    assert(deepIt->text()        == "\n    Text2");
    assert(deepIt->closingText() == "");

    siblingIt = deepIt;
    ++siblingIt;
    assert(siblingIt->isTag());
    assert(siblingIt->tagName()     == "br");
    assert(siblingIt->text()        == "<br>");
    assert(siblingIt->closingText() == "");
    assert(siblingIt->content(html) == "");
}

void test_attrs()
{
    std::string html(
R"(<DIV class="main" AttR2>
    Text
</div>)");
    ParserDom parser;
    Tree root = parser.parseTree(html);
    Tree::pre_order_iterator it = root.begin();
    ++it;
    assert(it->tagName() == "div");
    assert(it->parseAttributes() == 2);
    assert(it->hasAttribute("class"));
    assert(it->hasAttribute("attr2"));
    assert(it->hasAttribute("ATTR2"));
    assert(!it->hasAttribute("attr3"));
    std::string curAttr;
    assert(it->getAttribute("class", curAttr));
    assert(curAttr == "main");
    assert(it->getAttribute("attr2", curAttr));
    assert(curAttr.empty());
}

void test_links()
{
    std::string html(
R"(<div>
<a href="link1.html">link1</a>
<a href="link2.html">link2</a>
<p>Text <a href="link3.html">link3</a></p>
</div>)");
    ParserDom parser;
    Tree root = parser.parseTree(html);
    Tree::pre_order_iterator it = root.begin();

    std::vector<std::string> links;
    std::string href;
    std::for_each(it, root.end(), [&links, &href](Node &node)
    {
        if (node.isTag() 
                && (node.tagName() == "a")
                && (node.parseAttributes() > 0)
                && (node.attribute("href", href)))
            links.push_back(href);
    });
    assert(links.size() == 3);
    assert(links[0] == "link1.html");
    assert(links[1] == "link2.html");
    assert(links[2] == "link3.html");
}

int main()
{
    test0();
    test1();
    test_attrs();
    test_links();
    std::cout << "OK\n";
}