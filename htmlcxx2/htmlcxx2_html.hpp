// htmlcxx2.
// A simple non-validating parser written in C++.
//
// (c) 2005-2010 Davi de Castro Reis and Robson Braga Araújo
// (c) 2011 David Hoerl
// (c) 2017-01-24 Ruslan Zaporojets

#ifndef __HTML_PARSER_DOM_H__
#define __HTML_PARSER_DOM_H__

#include <cctype>
#include <cstring>
#if !(defined(WIN32) || defined(_WIN64)) || defined(__MINGW32__)
#include <strings.h>
#endif

#include <vector>
#include <map>
#include <string>
#include <utility>
#include <iostream>
#include <algorithm>

#include "kp_tree.hh"

namespace htmlcxx2 {
namespace HTML {

namespace detail {

    const char LITERAL_MODE_ELEM[][11] =
    {
        "\x06""script",
        "\x05""style",
        "\x03""xmp",
        "\x09""plaintext",
        "\x08""textarea"
    };

    template <typename It>
    inline It findNextQuote(It pos, It end, char quote)
    {
        while (pos != end && *pos != quote)
            ++pos;
        return pos;
    }

    template <>
    inline const char* findNextQuote(const char *pos, const char *end, char quote)
    {
        const char *found = reinterpret_cast<const char*>(memchr(pos, quote, end - pos));
        return found ? found : end;
    }

    template <class T>
    int icompare(const T *s1, const T *s2)
    {
        while (*s1 && *s2)
        {
            T ch1 = static_cast<T>(::tolower(*s1));
            T ch2 = static_cast<T>(::tolower(*s2));
            if (ch1 < ch2)
                return -1;
            else if (ch1 > ch2)
                return 1;
            ++s1;
            ++s2;
        }

        if (!(*s1))
            return !(*s2) ? 0 : -1;
        else
            return 1;
    }

    template <typename T>
    T toLower(const T &s)
    {
        T ret;
        std::transform(s.begin(), s.end(),
                std::back_insert_iterator<std::string>(ret), ::tolower);
        return ret;
    }

} // detail

//
// Node
//

class ParserSax;
class ParserDom;

class Node
{
public:
    enum Kind
    {
        NODE_END,
        NODE_ROOT,
        NODE_TAG,
        NODE_COMMENT,
        NODE_TEXT
    };

    Node() :
        tagName_(),
        text_(),
        closingText_(),
        offset_(0),
        length_(0),
        kind_(NODE_END),
        attributeKeys_(),
        attributeValues_(),
        attributesParsed_(false) { }

    Node(const std::string &tagName,
            const std::string &text,
            const std::string &closingText,
            size_t offset,
            size_t length,
            Kind kind) :
        tagName_(detail::toLower(tagName)),
        text_(text),
        closingText_(detail::toLower(closingText)),
        offset_(offset),
        length_(length),
        kind_(kind),
        attributeKeys_(),
        attributeValues_(),
        attributesParsed_(false) { }
    ~Node() { }

    const std::string& tagName() const     { return tagName_; }
    const std::string& text() const        { return text_; }
    const std::string& closingText() const { return closingText_; }    
    size_t offset() const                  { return offset_; }
    size_t length() const                  { return length_; }
    Kind kind() const                      { return kind_; }
    bool isEnd() const                     { return kind_ == NODE_END; }
    bool isRoot() const                    { return kind_ == NODE_ROOT; }
    bool isTag() const                     { return kind_ == NODE_TAG; }
    bool isComment() const                 { return kind_ == NODE_COMMENT; }
    bool isText() const                    { return kind_ == NODE_TEXT; }

    // TODO:
    /*void setTagName(const std::string& value)     { tagName_ = value; }
    void setText(const std::string &value)        { text_ = value; }
    void setClosingText(const std::string &value) { closingText_ = value; }
    void setOffset(size_t value)                  { offset_ = value; }
    void setLength(size_t value)                  { length_ = value; }
    void setKindTag()                             { kind_ = NODE_TAG; }
    void setKindComment()                         { kind_ = NODE_COMMENT; }
    void setKindText()                            { kind_ = NODE_TEXT; }*/

    size_t contentOffset() const;
    size_t contentLength() const;
    std::string content(const std::string &htmlSource) const;

    const std::vector<std::string>& attributeKeys() const   { return attributeKeys_; }
    const std::vector<std::string>& attributeValues() const { return attributeValues_; }
    bool hasAttribute(const std::string &key) const;
    bool attribute(const std::string &key, std::string &value) const;
    bool operator==(const Node &rhs) const;
    size_t parseAttributes();

protected:
    friend ParserSax;
    friend ParserDom;

    void addAttribute(const std::string &key, const std::string &value = "");

    std::string tagName_;
    std::string text_;
    std::string closingText_;
    size_t offset_;
    size_t length_;
    Kind kind_;
    std::vector<std::string> attributeKeys_;
    std::vector<std::string> attributeValues_;
    bool attributesParsed_;
};

inline size_t Node::contentOffset() const
{
    return !(isTag() || isRoot()) ? 0 : offset_ + text_.length();
}

inline size_t Node::contentLength() const
{
    return !(isTag() || isRoot()) ? 0 :
            length_ - text_.length() - closingText_.length();
}

inline std::string Node::content(const std::string &htmlSource) const
{
    return !(isTag() || isRoot()) ? std::string() :
            htmlSource.substr(contentOffset(), contentLength());
}

inline void Node::addAttribute(const std::string &key, const std::string &value)
{
    attributeKeys_.push_back(detail::toLower(key));
    attributeValues_.push_back(value);
}

inline bool Node::hasAttribute(const std::string &key) const
{
    for (size_t i = 0, l = attributeKeys_.size(); i < l; ++i)
        if (detail::icompare(attributeKeys_[i].c_str(), key.c_str()) == 0)
            return true;
    return false;
}

inline bool Node::attribute(const std::string &key, std::string &value) const
{
    for (size_t i = 0, l = attributeKeys_.size(); i < l; ++i)
        if (detail::icompare(attributeKeys_[i].c_str(), key.c_str()) == 0)
        {
            value = attributeValues_[i];
            return true;
        }
    return false;
}

inline bool Node::operator==(const Node &node) const
{
    if (kind_ != node.kind_)
        return false;
    if ((isRoot() && node.isRoot) || (isEnd() && node.isEnd())) // TODO:
        return true;
    if (isTag())
        return detail::icompare(tagName().c_str(), node.tagName().c_str()) == 0;
    else
        return detail::icompare(text().c_str(), node.text().c_str()) == 0;
}

//
// ParserSax
//

class ParserSax
{
    public:
        ParserSax() :
            currentOffset_(0),
            literal_(nullptr),
            cdata_(false) { }
        virtual ~ParserSax() { }
        void parse(const std::string &html);
        template <typename It> void parse(It begin, It end);

    protected:
        // Redefine this if you want to do some initialization before the parsing
        virtual void onBeginParsing() { }
        virtual void onFoundTag(Node& /*node*/, bool /*isClosingTag*/) { }
        virtual void onFoundText(Node& /*node*/) { }
        virtual void onFoundComment(Node& /*node*/) { }
        virtual void onEndParsing() { }

        template <typename It> void parse(It begin, It end, std::forward_iterator_tag);
        template <typename It> void parseTag(It begin, It end);
        template <typename It> void parseContent(It begin, It end);
        template <typename It> void parseComment(It begin, It end);
        template <typename It> It skipTag(It begin, It end);
        template <typename It> It skipComment(It begin, It end);

        size_t currentOffset_;
        const char *literal_;
        bool cdata_;
};

inline void ParserSax::parse(const std::string &html)
{
    parse(html.c_str(), html.c_str() + html.length());
}

template <typename It>
inline void ParserSax::parse(It begin, It end)
{
    parse(begin, end, typename std::iterator_traits<It>::iterator_category());
}

template <typename It>
void ParserSax::parse(It begin, It end, std::forward_iterator_tag)
{
    cdata_ = false;
    literal_ = 0;
    currentOffset_ = 0;
    onBeginParsing();

    while (begin != end)
    {
        (void)*begin; // This is for the multi_pass to release the buffer
        It c(begin);
        while (c != end)
        {
            // For some tags, the text inside it is considered literal and is
            // only closed for its </TAG> counterpart
            while (literal_)
            {
                while (c != end && *c != '<')
                    ++c;
                if (c == end)
                {
                    if (c != begin)
                        parseContent(begin, c);
                    goto DONE;
                }
                It end_text(c);
                ++c;
                if (*c == '/')
                {
                    ++c;
                    const char *l = literal_;
                    while (*l && ::tolower(*c) == *l)
                    {
                        ++c;
                        ++l;
                    }

                    // FIXME: Mozilla stops when it sees a /plaintext. Check
                    // other browsers and decide what to do
                    if (!*l && strcmp(literal_, "plaintext") != 0)
                    {
                        // matched all and is not tag plaintext
                        while (::isspace((unsigned char)*c))
                            ++c;
                        if (*c == '>')
                        {
                            ++c;
                            if (begin != end_text)
                                parseContent(begin, end_text);
                            literal_ = 0;
                            c = end_text;
                            begin = c;
                            break;
                        }
                    }
                }
                else if (*c == '!')
                {
                    // we may find a comment and we should support it
                    It e(c);
                    ++e;
                    if (e != end && *e == '-' && ++e != end && *e == '-')
                    {
                        ++e;
                        c = skipComment(e, end);
                    }
                }
            }

            if (*c == '<')
            {
                It d(c);
                ++d;
                if (d != end)
                {
                    if (::isalpha((unsigned char)*d))
                    {
                        // beginning of tag
                        if (begin != c)
                            parseContent(begin, c);

                        d = skipTag(d, end);
                        parseTag(c, d);

                        // continue from the end of the tag
                        c = d;
                        begin = c;
                        break;
                    }

                    if (*d == '/')
                    {
                        if (begin != c)
                            parseContent(begin, c);
                        It e(d);
                        ++e;
                        if (e != end && ::isalpha((unsigned char)*e))
                        {
                            // end of tag
                            d = skipTag(d, end);
                            parseTag(c, d);
                        }
                        else
                        {
                            // not a conforming end of tag, treat as comment
                            // as Mozilla does
                            d = skipTag(d, end);
                            parseComment(c, d);
                        }

                        // continue from the end of the tag
                        c = d;
                        begin = c;
                        break;
                    }

                    if (*d == '!')
                    {
                        // comment
                        if (begin != c)
                            parseContent(begin, c);
                        It e(d);
                        ++e;
                        if (e != end && *e == '-' && ++e != end && *e == '-')
                        {
                            ++e;
                            d = skipComment(e, end);
                        }
                        else
                            d = skipTag(d, end);
                        parseComment(c, d);

                        // continue from the end of the comment
                        c = d;
                        begin = c;
                        break;
                    }

                    if (*d == '?' || *d == '%')
                    {
                        // something like <?xml or <%VBSCRIPT
                        if (begin != c)
                            parseContent(begin, c);
                        d = skipTag(d, end);
                        parseComment(c, d);

                        // continue from the end of the comment
                        c = d;
                        begin = c;
                        break;
                    }
                }
            }
            c++;
        }

        // There may be some text in the end of the document
        if (begin != c)
        {
            parseContent(begin, c);
            begin = c;
        }
    }

DONE:
    onEndParsing();
    return;
}

template <typename It>
void ParserSax::parseComment(It begin, It pos)
{
    std::string comment(begin, pos);
    Node node("", comment, "", currentOffset_, comment.length(), Node::NODE_COMMENT);
    currentOffset_ += node.length();
    onFoundComment(node);
}

template <typename It>
void ParserSax::parseContent(It begin, It pos)
{
    std::string text(begin, pos);
    Node node("", text, "", currentOffset_, text.length(), Node::NODE_TEXT);
    currentOffset_ += node.length();
    onFoundText(node);
}

template <typename It>
void ParserSax::parseTag(It begin, It pos)
{
    It name_begin(begin);
    ++name_begin;
    bool isClosingTag = (*name_begin == '/');
    if (isClosingTag)
        ++name_begin;
    It name_end(name_begin);
    while ((name_end != pos) && ::isalnum((unsigned char)*name_end))
        ++name_end;
    std::string name(name_begin, name_end);

    if (!isClosingTag)
    {
        const size_t arrCount = sizeof(detail::LITERAL_MODE_ELEM) /
                sizeof(detail::LITERAL_MODE_ELEM[0]);
        const size_t tagLen = name.length();
        for (int i = 0; i < arrCount; ++i)
            if ((tagLen == static_cast<size_t>(detail::LITERAL_MODE_ELEM[i][0]))
                    && (detail::icompare(name.c_str(), &detail::LITERAL_MODE_ELEM[i][1]) == 0))
                {
                    literal_ = &detail::LITERAL_MODE_ELEM[i][1];
                    break;
                }
    }

    //by now, length is just the size of the tag
    std::string text(begin, pos);
    Node node(name, text, "", currentOffset_, text.length(), Node::NODE_TAG);
    currentOffset_ += node.length();
    onFoundTag(node, isClosingTag);
}

template <typename It>
It ParserSax::skipComment(It pos, It end)
{
    while (pos != end)
    {
        if (*pos++ == '-' && pos != end && *pos == '-')
        {
            It d(pos);
            while (++pos != end && ::isspace((unsigned char)*pos))
                ;
            if (pos == end || *pos++ == '>')
                break;
            pos = d;
        }
    }
    return pos;
}

template <typename It>
It ParserSax::skipTag(It pos, It end)
{
    while (pos != end && *pos != '>')
    {
        if (*pos != '=')
            ++pos;
        else
        {
            // found an attribute
            ++pos;
            while (pos != end && ::isspace((unsigned char)*pos))
                ++pos;
            if (pos == end)
                break;
            if (*pos == '\"' || *pos == '\'')
            {
                It save(pos);
                char quote = *pos++;
                pos = detail::findNextQuote(pos, end, quote);
                if (pos != end)
                    ++pos;
                else
                {
                    pos = save;
                    ++pos;
                }
            }
        }
    }
    if (pos != end)
        ++pos;
    return pos;
}

//
// ParserDom
//

typedef kp::tree<Node> Tree;

class ParserDom : public ParserSax
{
public:
    ParserDom() {}
    ~ParserDom() {}

    const Tree& parseTree(const std::string &html);
    const Tree& root() { return tree_; }

protected:
    virtual void onBeginParsing();
    virtual void onFoundTag(Node &node, bool isClosingTag);
    virtual void onFoundText(Node &node);
    virtual void onFoundComment(Node &node);
    virtual void onEndParsing();

    Tree tree_;
    Tree::iterator currIt_;
};

inline const Tree& ParserDom::parseTree(const std::string &html)
{
    parse(html);
    return root();
}

inline void ParserDom::onBeginParsing()
{
    tree_.clear();
    Tree::iterator top = tree_.begin();
    Node node("", "", "" , 0, 0, Node::NODE_ROOT);
    currIt_ = tree_.insert(top, node);
}

inline void ParserDom::onEndParsing()
{
    Tree::iterator top = tree_.begin();
    top->length_ = currentOffset_;
}

inline void ParserDom::onFoundComment(Node &node)
{
    //Add child content node, but do not update current state
    onFoundText(node);
}

inline void ParserDom::onFoundText(Node &node)
{
    //Add child content node, but do not update current state
    tree_.append_child(currIt_, node);
}

//
// Utils
//

template <typename It>
inline It findTag(It it, It end, const std::string &tag)
{
    return std::find_if(it, end, [&tag](const Node &node)
    {
        return node.isTag()
            && (detail::icompare(node.tagName().c_str(), tag.c_str()) == 0);
    });
}

template <typename It>
inline It rfindTag(It it, It rend, const std::string &tag)
{
    while (it != rend)
    {
        if (it->isTag() && (detail::icompare(
                it->tagName().c_str(), tag.c_str()) == 0))
            return it;
        --it;
    }
    return rend;
}

std::string decodeEntities(const std::string &str);

#ifdef _DEBUG
std::ostream& operator<<(std::ostream &stream, const Tree &tr);
#endif

} }

#endif
