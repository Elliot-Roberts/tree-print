#include<string>
#include<sstream>
#include<tuple>
#include<algorithm>
#include<vector>
#include<unordered_map>
#include<utility>
#include<iostream>
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string BLUE = "\033[34m";
const std::string RESET = "\033[0m";

const std::string DELETE_COLOR = RED;
const std::string ADD_COLOR = GREEN;

class StringBuff {
    public:
        const std::size_t height;
        const std::size_t width;
        StringBuff(std::size_t height, std::size_t width, std::string fill = " "): 
            height(height), width(width), buff(height*width, fill) {}
        std::string & at(std::size_t row, std::size_t col) {
            return buff[row * width + col];
        }
        friend std::ostream & operator<<(std::ostream &os, const StringBuff &sb) {
            size_t buff_size = sb.height * sb.width;
            for (std::size_t i = 0; i < buff_size; i += sb.width) {
                for (std::size_t j = 0; j < sb.width; ++j) {
                    os << sb.buff[i+j];
                }
                os << std::endl;
            }
            return os;
        }
    private:
        std::vector<std::string> buff;
};
class BuffView {
    public:
        BuffView(StringBuff &buff, std::size_t base_row = 0, std::size_t base_col = 0):
            buff(buff), base_row(base_row), base_col(base_col) {}
        BuffView offset(std::size_t rows, std::size_t cols) {
            return BuffView(buff, base_row + rows, base_col + cols);
        }
        std::string & at(std::size_t row, std::size_t col) {
            return buff.at(base_row + row, base_col + col);
        }
    private:
        StringBuff &buff;
        std::size_t base_row;
        std::size_t base_col;
};
struct Span {
    std::size_t offset;
    std::size_t size;
    Span(std::size_t offset, std::size_t size): offset(offset), size(size) {}
    std::size_t end() {
        return offset + size;
    }
};
template<std::size_t h = 1>
class SimpleLineDrawer {
    public:
        static const std::size_t row_inc = 1 + h;
        static std::size_t calc_height(std::size_t nodes) {
            return nodes + (row_inc - 1) * (nodes - 1);
        }
        static void draw(BuffView bv,
                          Span lroot,
                          Span root,
                          Span rroot) {
            if (lroot.size) {
                auto lroot_mid = lroot.offset + ((lroot.size + 1) / 2 - lroot.size%2);
                bv.at(0, lroot_mid) = "╭";
                for (std::size_t i = 1; i <= h; ++i) {
                    bv.at(i, lroot_mid) = "│";
                }
                for (auto i = lroot_mid + 1; i < root.offset; ++i) {
                    bv.at(0, i) = "─";
                }
            }
            if (rroot.size) {
                auto rroot_mid = rroot.offset + (rroot.size / 2 - 1 + rroot.size%2);
                bv.at(0, rroot_mid) = "╮";
                for (std::size_t i = 1; i <= h; ++i) {
                    bv.at(i, rroot_mid) = "│";
                }
                for (auto i = root.end(); i < rroot_mid; ++i) {
                    bv.at(0, i) = "─";
                }
            }
        }
};

template<std::size_t h = 0>
class SquigleLineDrawer {
    public:
        static const std::size_t row_inc = 2 + 2*h;
        static std::size_t calc_height(std::size_t nodes) {
            return nodes + row_inc * (nodes - 1);
        }
        static void draw(BuffView bv,
                          Span lroot,
                          Span root,
                          Span rroot) {
            if (lroot.size) {
                auto lroot_mid = lroot.offset + ((lroot.size + 1) / 2 - lroot.size%2);
                auto mid_left = root.offset;
                for (std::size_t i = 1; i <= h; ++i) {
                    bv.at(i, mid_left) = "│";
                }
                auto bend_row = h+1;
                bv.at(bend_row, mid_left) = "╯";
                for (auto i = lroot_mid+1; i < root.offset; ++i) {
                    bv.at(bend_row, i) = "─";
                }
                bv.at(bend_row, lroot_mid) = "╭";
                for (std::size_t i = bend_row+1; i < row_inc; ++i) {
                    bv.at(i, lroot_mid) = "│";
                }
            }
            if (rroot.size) {
                auto rroot_mid = rroot.offset + (rroot.size / 2 - 1 + rroot.size%2);
                auto mid_right = root.end()-1;
                for (std::size_t i = 1; i <= h; ++i) {
                    bv.at(i, mid_right) = "│";
                }
                auto bend_row = h+1;
                bv.at(bend_row, mid_right) = "╰";
                for (auto i = mid_right+1; i < rroot_mid; ++i) {
                    bv.at(bend_row, i) = "─";
                }
                bv.at(bend_row, rroot_mid) = "╮";
                for (std::size_t i = bend_row+1; i < row_inc; ++i) {
                    bv.at(i, rroot_mid) = "│";
                }
            }
        }
};

template<typename Node>
class WrappedTree {
    using Data = decltype(Node::data);
    struct Wrap {
        const Node* n;
        const Node* parent;
        const Node* left;
        const Node* right;

        std::string display;
        std::string color;
        std::size_t width;
        std::size_t height;
        
        Wrap() = delete;

        Wrap(
        const Node* n,
        const Node* parent,
        const Node* left,
        const Node* right,
        std::string display,
        std::string color,
        std::size_t width,
        std::size_t height
        ):
        n(n),
        parent(parent),
        left(left),
        right(right),
        display(display),
        color(color),
        width(width),
        height(height)
        {}
    };
    std::unordered_map<const Node*, Wrap> wrap_map;
    const Node* root;
    void wrap(const Node* n, const Node* parent) {
        if (n == nullptr) return;
        wrap(n->left, n);
        wrap(n->right, n);
        const Wrap &lw = wrap_map.at(n->left);
        const Wrap &rw = wrap_map.at(n->right);

        std::stringstream ss;
        ss << n->data;
        std::string display = ss.str();
        std::size_t self_width = display.size() + 2;
        std::size_t width = self_width + lw.width + rw.width;
        std::size_t height = 1 + std::max(lw.height, rw.height);
        wrap_map.emplace(n, Wrap(
            n, 
            parent,
            n->left,
            n->right,
            display,
            "",
            width,
            height
        ));
    };

    template<class LinkDrawer>
    std::pair<std::size_t, Span> draw(const Node* n, BuffView bv) const {
        if (n == nullptr) return {0, Span(0, 0)};
        Wrap w = wrap_map.at(n);
        std::size_t self_width = w.display.size() + 2;

        auto [lwidth, lroot] = draw<LinkDrawer>(w.left, bv.offset(LinkDrawer::row_inc, 0));
        std::size_t rstart = lwidth + self_width;
        auto [rwidth, rroot] = draw<LinkDrawer>(w.right, bv.offset(LinkDrawer::row_inc, rstart));

        rroot.offset += rstart;
        Span self_span(lwidth, self_width);
        LinkDrawer::draw(bv, lroot, self_span, rroot);

        bv.at(0, lwidth) = "[" + w.color;
        for (std::size_t i = 0; i < w.display.size(); ++i) {
            bv.at(0, lwidth+1+i) = w.display[i];
        }
        bv.at(0, lwidth+1+w.display.size()) = RESET + "]";

        return {lwidth + self_width + rwidth, self_span};
    }
    public:
        WrappedTree(const Node* n): root(n) {
            wrap_map.emplace(nullptr, Wrap(nullptr, nullptr, nullptr, nullptr, "", "", 0, 0));
            wrap(n, nullptr);
        }
        template<class LinkDrawer = SimpleLineDrawer<1>>
        StringBuff draw() const {
            const Wrap &wroot = wrap_map.at(root);
            StringBuff sb(LinkDrawer::calc_height(wroot.height), wroot.width);
            draw<LinkDrawer>(root, sb);
            return sb;
        }
        void color_node(const Node* n, std::string color) {
            wrap_map.at(n).color = color;
        }
        std::pair<WrappedTree, WrappedTree> compare_to(const WrappedTree &other) const {
            WrappedTree oldt(*this);
            WrappedTree newt(other);
            for (auto & p : oldt.wrap_map) {
                const Node* n = p.first;
                Wrap &wr = p.second;
                const auto found = newt.wrap_map.find(n);
                if (found == other.wrap_map.end()) {
                    wr.color = DELETE_COLOR;
                }
            }
            for (auto & p : newt.wrap_map) {
                const Node* n = p.first;
                Wrap &wr = p.second;
                const auto found = oldt.wrap_map.find(n);
                if (found == other.wrap_map.end()) {
                    wr.color = ADD_COLOR;
                }
            }
            return {oldt, newt};
        }
        friend std::ostream & operator<<(std::ostream &os, const WrappedTree<Node> &wt) {
            return os << wt.draw();
        }
};
