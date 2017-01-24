#ifndef __HTML_PARSER_DOM_H__
#define __HTML_PARSER_DOM_H__

#include <cctype>
#include <cstring>
#if !defined(WIN32) || defined(__MINGW32__)
#include <strings.h>
#endif

#include <vector>
#include <map>
#include <string>
#include <utility>
#include <iostream>
#include <algorithm>

#if defined(WIN32) && !defined(__MINGW32__)
#include <cctype>
#ifndef strcasecmp
# define strcasecmp _stricmp
#endif
#ifndef strncasecmp
# define strncasecmp _strnicmp
#endif
#ifndef snprintf
# define snprintf(buf, n, format, ...) _snprintf_s(buf, n, format, __VA_ARGS__)
#endif
#endif // WIN32

#include "kp_tree.hh"

namespace htmlcxx {
namespace HTML {

//
// Node
//

class Node
{
public:
	Node() {}
	//Node(const Node &rhs); //uses default
	~Node() {}

	inline void text(const std::string& text) { this->mText = text; }
	inline const std::string& text() const { return this->mText; }

	inline unsigned int contentOffset() const { return static_cast<unsigned int>(this->mOffset + this->mText.length()); }
	inline unsigned int contentLength() const { return static_cast<unsigned int>(this->mLength - this->mText.length() - this->mClosingText.length()); }
	inline std::string content(const std::string& html) const { return html.substr(this->contentOffset(), this->contentLength()); }

	inline void closingText(const std::string &text) { this->mClosingText = text; }
	inline const std::string& closingText() const { return mClosingText; }

	inline void offset(unsigned int offset) { this->mOffset = offset; }
	inline unsigned int offset() const { return this->mOffset; }

	inline void length(unsigned int length) { this->mLength = length; }
	inline unsigned int length() const { return this->mLength; }

	inline void tagName(const std::string& tagname) { this->mTagName = tagname; }
	inline const std::string& tagName() const { return this->mTagName; }

	bool isTag() const { return this->mIsHtmlTag; }
	void isTag(bool is_html_tag){ this->mIsHtmlTag = is_html_tag; }

	bool isComment() const { return this->mComment; }
	void isComment(bool comment){ this->mComment = comment; }

	std::pair<bool, std::string> attribute(const std::string &attr) const;

	operator std::string() const;
	std::ostream &operator<<(std::ostream &stream) const;

	const std::map<std::string, std::string>& attributes() const { return this->mAttributes; }
	void parseAttributes();

	bool operator==(const Node &rhs) const;

protected:
	std::string mText;
	std::string mClosingText;
	unsigned int mOffset;
	unsigned int mLength;
	std::string mTagName;
	std::map<std::string, std::string> mAttributes;
	bool mIsHtmlTag;
	bool mComment;
};

inline std::pair<bool, std::string> Node::attribute(const std::string &attr) const
{
	std::map<std::string, std::string>::const_iterator i = this->mAttributes.find(attr);
	if (i != this->mAttributes.end()) {
		return make_pair(true, i->second);
	}
	else {
		return make_pair(false, std::string());
	}
}

inline bool Node::operator==(const Node &n) const
{
	if (!isTag() || !n.isTag()) return false;
	return !(strcasecmp(tagName().c_str(), n.tagName().c_str()));
}

inline Node::operator std::string() const {
	if (isTag()) return this->tagName();
	return this->text();
}

inline std::ostream &Node::operator<<(std::ostream &stream) const {
	stream << (std::string)(*this);
	return stream;
}

//
// ParserSax
//

class ParserSax
{
	public:
		ParserSax() : mpLiteral(0), mCdata(false) {}
		virtual ~ParserSax() {}

		/** Parse the html code */
		void parse(const std::string &html);

		template <typename _Iterator>
		void parse(_Iterator begin, _Iterator end);

	protected:
		// Redefine this if you want to do some initialization before
		// the parsing
		virtual void beginParsing() {}

		virtual void foundTag(Node /*node*/, bool /*isEnd*/) {}
		virtual void foundText(Node node) {}
		virtual void foundComment(Node node) {}

		virtual void endParsing() {}


		template <typename _Iterator>
		void parse(_Iterator &begin, _Iterator &end,
				std::forward_iterator_tag);

		template <typename _Iterator>
		void parseHtmlTag(_Iterator b, _Iterator c);

		template <typename _Iterator>
		void parseContent(_Iterator b, _Iterator c);

		template <typename _Iterator>
		void parseComment(_Iterator b, _Iterator c);

		template <typename _Iterator>
		_Iterator skipHtmlTag(_Iterator ptr, _Iterator end);
				
		template <typename _Iterator>
		_Iterator skipHtmlComment(_Iterator ptr, _Iterator end);

		unsigned long mCurrentOffset;
		const char *mpLiteral;
		bool mCdata;
};

inline void ParserSax::parse(const std::string &html)
{
	parse(html.c_str(), html.c_str() + html.length());
}

template <typename _Iterator>
inline void ParserSax::parse(_Iterator begin, _Iterator end)
{
	parse(begin, end, typename std::iterator_traits<_Iterator>::iterator_category());
}

template <typename _Iterator>
void ParserSax::parse(_Iterator &begin, _Iterator &end, std::forward_iterator_tag)
{
	typedef _Iterator iterator;
	//	std::cerr << "Parsing forward_iterator" << std::endl;
	mCdata = false;
	mpLiteral = 0;
	mCurrentOffset = 0;
	this->beginParsing();

	//	DEBUGP("Parsed text\n");

	while (begin != end)
	{
		(void)*begin; // This is for the multi_pass to release the buffer
		iterator c(begin);

		while (c != end)
		{
			// For some tags, the text inside it is considered literal and is
			// only closed for its </TAG> counterpart
			while (mpLiteral)
			{
				//				DEBUGP("Treating literal %s\n", mpLiteral);
				while (c != end && *c != '<') ++c;

				if (c == end) {
					if (c != begin) this->parseContent(begin, c);
					goto DONE;
				}

				iterator end_text(c);
				++c;

				if (*c == '/')
				{
					++c;
					const char *l = mpLiteral;
					while (*l && ::tolower(*c) == *l)
					{
						++c;
						++l;
					}

					// FIXME: Mozilla stops when it sees a /plaintext. Check
					// other browsers and decide what to do
					if (!*l && strcmp(mpLiteral, "plaintext") != 0)
					{
						// matched all and is not tag plaintext
						while (isspace((unsigned char)*c)) ++c;

						if (*c == '>')
						{
							++c;
							if (begin != end_text)
								this->parseContent(begin, end_text);
							mpLiteral = 0;
							c = end_text;
							begin = c;
							break;
						}
					}
				}
				else if (*c == '!')
				{
					// we may find a comment and we should support it
					iterator e(c);
					++e;

					if (e != end && *e == '-' && ++e != end && *e == '-')
					{
						//						DEBUGP("Parsing comment\n");
						++e;
						c = this->skipHtmlComment(e, end);
					}

					//if (begin != end_text)
					//this->parseContent(begin, end_text, end);

					//this->parseComment(end_text, c, end);

					// continue from the end of the comment
					//begin = c;
				}
			}

			if (*c == '<')
			{
				iterator d(c);
				++d;
				if (d != end)
				{
					if (isalpha((unsigned char)*d))
					{
						// beginning of tag
						if (begin != c)
							this->parseContent(begin, c);

						//						DEBUGP("Parsing beginning of tag\n");
						d = this->skipHtmlTag(d, end);
						this->parseHtmlTag(c, d);

						// continue from the end of the tag
						c = d;
						begin = c;
						break;
					}

					if (*d == '/')
					{
						if (begin != c)
							this->parseContent(begin, c);

						iterator e(d);
						++e;
						if (e != end && isalpha((unsigned char)*e))
						{
							// end of tag
							//							DEBUGP("Parsing end of tag\n");
							d = this->skipHtmlTag(d, end);
							this->parseHtmlTag(c, d);
						}
						else
						{
							// not a conforming end of tag, treat as comment
							// as Mozilla does
							//							DEBUGP("Non conforming end of tag\n");
							d = this->skipHtmlTag(d, end);
							this->parseComment(c, d);
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
							this->parseContent(begin, c);

						iterator e(d);
						++e;

						if (e != end && *e == '-' && ++e != end && *e == '-')
						{
							//							DEBUGP("Parsing comment\n");
							++e;
							d = this->skipHtmlComment(e, end);
						}
						else
						{
							d = this->skipHtmlTag(d, end);
						}

						this->parseComment(c, d);

						// continue from the end of the comment
						c = d;
						begin = c;
						break;
					}

					if (*d == '?' || *d == '%')
					{
						// something like <?xml or <%VBSCRIPT
						if (begin != c)
							this->parseContent(begin, c);

						d = this->skipHtmlTag(d, end);

						this->parseComment(c, d);

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
			this->parseContent(begin, c);
			begin = c;
		}
	}

DONE:
	this->endParsing();
	return;
}

template <typename _Iterator>
void ParserSax::parseComment(_Iterator b, _Iterator c)
{
	Node com_node;
	//FIXME: set_tagname shouldn't be needed, but first I must check
	//legacy code
	std::string comment(b, c);
	com_node.tagName(comment);
	com_node.text(comment);
	com_node.offset(mCurrentOffset);
	com_node.length((unsigned int)comment.length());
	com_node.isTag(false);
	com_node.isComment(true);

	mCurrentOffset += com_node.length();

	// Call callback method
	this->foundComment(com_node);
}

template <typename _Iterator>
void ParserSax::parseContent(_Iterator b, _Iterator c)
{
	Node txt_node;
	//FIXME: set_tagname shouldn't be needed, but first I must check
	//legacy code
	std::string text(b, c);
	txt_node.tagName(text);
	txt_node.text(text);
	txt_node.offset(mCurrentOffset);
	txt_node.length((unsigned int)text.length());
	txt_node.isTag(false);
	txt_node.isComment(false);

	mCurrentOffset += txt_node.length();

	// Call callback method
	this->foundText(txt_node);
}

struct literal_tag
{
	int len;
	const char* str;
	int is_cdata;
};

extern literal_tag literal_mode_elem[];

template <typename _Iterator>
void ParserSax::parseHtmlTag(_Iterator b, _Iterator c)
{
	_Iterator name_begin(b);
	++name_begin;
	bool is_end_tag = (*name_begin == '/');
	if (is_end_tag) ++name_begin;

	_Iterator name_end(name_begin);
	while (name_end != c && isalnum((unsigned char)*name_end))
	{
		++name_end;
	}

	std::string name(name_begin, name_end);

	if (!is_end_tag)
	{
		std::string::size_type tag_len = name.length();
		for (int i = 0; literal_mode_elem[i].len; ++i)
		{
			if (tag_len == (std::string::size_type)literal_mode_elem[i].len)
			{
				if (!strcasecmp(name.c_str(), literal_mode_elem[i].str))
				{
					mpLiteral = literal_mode_elem[i].str;
					break;
				}
			}
		}
	}

	Node tag_node;
	//by now, length is just the size of the tag
	std::string text(b, c);
	tag_node.length(static_cast<unsigned int>(text.length()));
	tag_node.tagName(name);
	tag_node.text(text);
	tag_node.offset(mCurrentOffset);
	tag_node.isTag(true);
	tag_node.isComment(false);

	mCurrentOffset += tag_node.length();

	this->foundTag(tag_node, is_end_tag);
}

template <typename _Iterator>
_Iterator ParserSax::skipHtmlComment(_Iterator c, _Iterator end)
{
	while (c != end) {
		if (*c++ == '-' && c != end && *c == '-')
		{
			_Iterator d(c);
			while (++c != end && isspace((unsigned char)*c));
			if (c == end || *c++ == '>') break;
			c = d;
		}
	}

	return c;
}

template <typename _Iterator>
inline _Iterator find_next_quote(_Iterator c, _Iterator end, char quote)
{
	while (c != end && *c != quote)
		++c;
	return c;
}

template <>
inline const char* find_next_quote(const char *c, const char *end, char quote)
{
	const char *d = reinterpret_cast<const char*>(memchr(c, quote, end - c));
	if (d) return d;
	else return end;
}

template <typename _Iterator>
_Iterator ParserSax::skipHtmlTag(_Iterator c, _Iterator end)
{
	while (c != end && *c != '>')
	{
		if (*c != '=')
			++c;
		else
		{ // found an attribute
			++c;
			while (c != end && isspace((unsigned char)*c)) ++c;

			if (c == end) break;

			if (*c == '\"' || *c == '\'')
			{
				_Iterator save(c);
				char quote = *c++;
				c = find_next_quote(c, end, quote);
				//				while (c != end && *c != quote) ++c;
				//				c = static_cast<char*>(memchr(c, quote, end - c));
				if (c != end)
				{
					++c;
				}
				else
				{
					c = save;
					++c;
				}
			}
		}
	}

	if (c != end)
		++c;
	return c;
}

//
// ParserDom
//

class ParserDom : public ParserSax
{
public:
	ParserDom() {}
	~ParserDom() {}

	const kp::tree<Node> &parseTree(const std::string &html);
	const kp::tree<Node> &getTree() { return mHtmlTree; }

protected:
	virtual void beginParsing();

	virtual void foundTag(Node node, bool isEnd);
	virtual void foundText(Node node);
	virtual void foundComment(Node node);

	virtual void endParsing();

	kp::tree<Node> mHtmlTree;
	kp::tree<Node>::iterator mCurrentState;
};

inline const kp::tree<HTML::Node>& ParserDom::parseTree(const std::string &html)
{
	this->parse(html);
	return this->getTree();
}

inline void ParserDom::beginParsing()
{
	mHtmlTree.clear();
	kp::tree<HTML::Node>::iterator top = mHtmlTree.begin();
	HTML::Node lambda_node;
	lambda_node.offset(0);
	lambda_node.length(0);
	lambda_node.isTag(true);
	lambda_node.isComment(false);
	mCurrentState = mHtmlTree.insert(top, lambda_node);
}

inline void ParserDom::endParsing()
{
	kp::tree<HTML::Node>::iterator top = mHtmlTree.begin();
	top->length(mCurrentOffset);
}

inline void ParserDom::foundComment(Node node)
{
	//Add child content node, but do not update current state
	mHtmlTree.append_child(mCurrentState, node);
}

inline void ParserDom::foundText(Node node)
{
	//Add child content node, but do not update current state
	mHtmlTree.append_child(mCurrentState, node);
}

//
// Utils
//

std::string decode_entities(const std::string &str);

#ifdef _DEBUG
std::ostream& operator<<(std::ostream &stream, const kp::tree<Node> &tr);
#endif

} }

#endif
