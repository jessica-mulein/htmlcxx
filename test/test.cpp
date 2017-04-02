// htmlcxx2.
// A simple non-validating parser written in C++.
//
// (c) 2017-01-24 Ruslan Zaporojets

#define CATCH_CONFIG_MAIN
#include <htmlcxx2/htmlcxx2_html.hpp>
#include "catch.hpp"

using namespace htmlcxx2::HTML;

TEST_CASE("begin, end, emty")
{
    // [head]-[root]
    //          |
    //        [end]
    std::string html;
    ParserDom parser;
    // -> head (null)
    Tree domTree = parser.parseTree(html);
    // -> root node (null)
    auto it = domTree.begin();
    auto endIt = domTree.end();
    REQUIRE(it->isRoot());
    REQUIRE(it.number_of_children() == 0);
    ++it;
    // -> end node
    REQUIRE(it->isEnd());
    REQUIRE(it == endIt);
    REQUIRE(it.node == endIt.node);
    ++it;
    REQUIRE(!it);


    // [head]-[root]
    //          |
    //        [DIV]
    //          |
    //        [end]
    html = "<div/>";                  
    // -> head (null)
    domTree = parser.parseTree(html);
    // -> root node (null)
    it = domTree.begin();
    REQUIRE(it->isRoot());
    REQUIRE(it.number_of_children() == 1);
    ++it;
    // -> DIV node
    REQUIRE(it->isTag());
    ++it;
    // -> end node
    REQUIRE(it->isEnd());
    ++it;
    REQUIRE(!it);

    // -> end node
    auto rit = domTree.end();    
    REQUIRE(rit->isEnd());
    // -> DIV node
    --rit;
    REQUIRE(rit->isTag());
    // -> root node (null)
    --rit;
    REQUIRE(rit->isRoot());
    // -> end node
    --rit;
    REQUIRE(rit->isEnd());
    --rit;
    REQUIRE(!rit);
}

TEST_CASE("tree")
{
    // [head]-[root]
    //         /|\
    //   [DIV][Text2][p]
    //    |          /|\
    // [Text] [Text3][BR][ ][comment][\n]
    std::string html(
R"(<div id="1">
    Text
</div>Text2
<p>
    Text3<br> <!-- Comment -->
</p>)");

    ParserDom parser;
    Tree domTree = parser.parseTree(html);
    Tree::pre_order_iterator deepIt = domTree.begin();  // -> root node (null)
    REQUIRE(deepIt->isRoot());
    REQUIRE(deepIt->tagName()     == "");
    REQUIRE(deepIt->text()        == "");
    REQUIRE(deepIt->closingText() == "");
    REQUIRE(deepIt->content(html) == html);
    
    Tree::sibling_iterator siblingIt = deepIt.begin();  // -> DIV node
    REQUIRE(siblingIt->isTag());
    REQUIRE(siblingIt->tagName()     == "div");
    REQUIRE(siblingIt->text()        == "<div id=\"1\">");
    REQUIRE(siblingIt->closingText() == "</div>");
    REQUIRE(siblingIt->content(html) == "\n    Text\n");
    // or
    ++deepIt;                                           // -> DIV node
    REQUIRE(deepIt->isTag());
    REQUIRE(deepIt->tagName()     == "div");
    REQUIRE(deepIt->text()        == "<div id=\"1\">");
    REQUIRE(deepIt->closingText() == "</div>");
    REQUIRE(deepIt->content(html) == "\n    Text\n");
    
    ++siblingIt;                                        // -> Text2 node
    REQUIRE(siblingIt->isText());
    REQUIRE(siblingIt->tagName()     == "");
    REQUIRE(siblingIt->text()        == "Text2\n");
    REQUIRE(siblingIt->closingText() == "");
    REQUIRE(siblingIt->content(html) == "");

    ++siblingIt;                                        // -> P node
    REQUIRE(siblingIt->isTag());
    REQUIRE(siblingIt->tagName()     == "p");
    REQUIRE(siblingIt->text()        == "<p>");
    REQUIRE(siblingIt->closingText() == "</p>");
    REQUIRE(siblingIt->content(html) == "\n    Text3<br> <!-- Comment -->\n");

    ++deepIt;                                           // -> Text node
    REQUIRE(deepIt->isText());
    REQUIRE(deepIt->tagName()     == "");
    REQUIRE(deepIt->text()        == "\n    Text\n");
    REQUIRE(deepIt->closingText() == "");
    REQUIRE(deepIt->content(html) == "");

    deepIt = siblingIt.begin();                         // -> Text3 node
    REQUIRE(deepIt->isText());
    REQUIRE(deepIt->tagName()     == "");
    REQUIRE(deepIt->text()        == "\n    Text3");
    REQUIRE(deepIt->closingText() == "");
    REQUIRE(deepIt->content(html) == "");

    siblingIt = deepIt;                                 // -> Text3 node
    ++siblingIt;                                        // -> BR node
    REQUIRE(siblingIt->isTag());
    REQUIRE(siblingIt->tagName()     == "br");
    REQUIRE(siblingIt->text()        == "<br>");
    REQUIRE(siblingIt->closingText() == "");
    REQUIRE(siblingIt->content(html) == "");

    ++siblingIt;                                        // -> ' ' node
    REQUIRE(siblingIt->isText());
    REQUIRE(siblingIt->tagName() == "");
    REQUIRE(siblingIt->text() == " ");
    REQUIRE(siblingIt->closingText() == "");
    REQUIRE(siblingIt->content(html) == "");

    ++siblingIt;                                        // -> comment node
    REQUIRE(siblingIt->isComment());
    REQUIRE(siblingIt->tagName() == "");
    REQUIRE(siblingIt->text() == "<!-- Comment -->");
    REQUIRE(siblingIt->closingText() == "");
    REQUIRE(siblingIt->content(html) == "");

    ++siblingIt;                                        // -> \n node
    REQUIRE(siblingIt->isText());
    REQUIRE(siblingIt->tagName() == "");
    REQUIRE(siblingIt->text() == "\n");
    REQUIRE(siblingIt->closingText() == "");
    REQUIRE(siblingIt->content(html) == "");

    REQUIRE(siblingIt);
    ++siblingIt;    
    REQUIRE(siblingIt == siblingIt.end());
    REQUIRE(!siblingIt);
}

TEST_CASE("attrs")
{
    std::string html(
R"(<DIV class="main" AttR2>
    Text
</div>)");
    ParserDom parser;
    Tree domTree = parser.parseTree(html);
    Tree::pre_order_iterator it = domTree.begin();
    ++it;
    REQUIRE(it->tagName() == "div");
    REQUIRE(it->parseAttributes() == 2);
    REQUIRE(it->hasAttribute("class"));
    REQUIRE(it->hasAttribute("attr2"));
    REQUIRE(it->hasAttribute("ATTR2"));
    REQUIRE(!it->hasAttribute("attr3"));
    std::string curAttr;
    REQUIRE(it->attribute("class", curAttr));
    REQUIRE(curAttr == "main");
    REQUIRE(it->attribute("attr2", curAttr));
    REQUIRE(curAttr.empty());
}

TEST_CASE("links")
{
    std::string html(
R"(<div>
<a href="link1.html">link1</a>
<a href="link2.html">link2</a>
<p>Text <a href="link3.html">link3</a></p>
</div>)");
    ParserDom parser;
    Tree domTree = parser.parseTree(html);
    Tree::pre_order_iterator it = domTree.begin();

    std::vector<std::string> links;
    std::string href;
    std::for_each(it, domTree.end(), [&links, &href](Node &node)
    {
        if (node.isTag() 
                && (node.tagName() == "a")
                && (node.parseAttributes() > 0)
                && (node.attribute("href", href)))
            links.push_back(href);
    });
    REQUIRE(links.size() == 3);
    REQUIRE(links[0] == "link1.html");
    REQUIRE(links[1] == "link2.html");
    REQUIRE(links[2] == "link3.html");
}

TEST_CASE("find tag")
{
    std::string html(
R"(<div>
<p>Text_1</p>
<div>
<span>Text</span>
<span>Text</span>
<p>Text2</p>
</div>
<div>
<span>Text</span>
<span>Text</span>
<p>Text3</p>
</div>
</div>)");
    ParserDom parser;
    Tree domTree = parser.parseTree(html);
    Tree::pre_order_iterator it = domTree.begin();
    Tree::pre_order_iterator endIt = domTree.end();

    REQUIRE((it = findTag(it, endIt, "p")) != endIt);
    REQUIRE(it->content(html) == "Text_1");
    ++it;
    REQUIRE((it = findTag(it, endIt, "p")) != endIt);
    REQUIRE(it->content(html) == "Text2");
    ++it;
    REQUIRE((it = findTag(it, endIt, "p")) != endIt);
    REQUIRE(it->content(html) == "Text3");
    ++it;
    REQUIRE((it = findTag(it, endIt, "p")) == endIt);

    it = domTree.end();
    endIt = domTree.begin();
    REQUIRE((it = rfindTag(it, endIt, "p")) != endIt);
    REQUIRE(it->content(html) == "Text3");
    --it;
    REQUIRE((it = rfindTag(it, endIt, "p")) != endIt);
    REQUIRE(it->content(html) == "Text2");
    --it;
    REQUIRE((it = rfindTag(it, endIt, "p")) != endIt);
    REQUIRE(it->content(html) == "Text_1");
    --it;
    REQUIRE((it = rfindTag(it, endIt, "p")) == endIt);
}


