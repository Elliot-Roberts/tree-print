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
class StringBuff {
    public:
        StringBuff(std::size_t width, std::size_t height, std::string fill = " "): 
            width(width), height(height), buff(width*height, fill) {}
        std::string & at(std::size_t row, std::size_t col) {
            return buff[row * width + col];
        }
        friend std::ostream & operator<<(std::ostream &os, const StringBuff &sb);
    private:
        std::size_t width;
        std::size_t height;
        std::vector<std::string> buff;
};
std::ostream & operator<<(std::ostream &os, const StringBuff &sb) {
    size_t buff_size = sb.width * sb.height;
    for (std::size_t i = 0; i < buff_size; i += sb.width) {
        for (std::size_t j = 0; j < sb.width; ++j) {
            os << sb.buff[i+j];
        }
        os << std::endl;
    }
    return os;
}
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
};
// class LinkDrawer {
//     public:
//         const std::size_t vertical_space = 1;
//         virtual void draw(BuffView bv,
//                           std::size_t lwidth,
//                           std::size_t lroot,
//                           std::size_t midwidth,
//                           std::size_t rwidth,
//                           std::size_t rroot) {
// 
//         }
// };

template<typename Node>
class TreeHelper {
    using Data = decltype(Node::data);
    class WrappedTree {
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

        std::pair<std::size_t, Span> draw(const Node* n, BuffView bv) {
            if (n == nullptr) return {0, Span(0, 0)};
            Wrap w = wrap_map.at(n);
            auto [lwidth, lroot_span] = draw(w.left, bv.offset(2, 0));
            if (lwidth) {
                std::size_t lroot = lroot_span.offset + ((lroot_span.size + 1) / 2 - lroot_span.size%2);
                bv.at(0, lroot) = "╭";
                bv.at(1, lroot) = "│";
                for (std::size_t i = lroot + 1; i < lwidth; ++i) {
                    bv.at(0, i) = "─";
                }
            }
            bv.at(0, lwidth) = "[" + w.color;
            for (std::size_t i = 0; i < w.display.size(); ++i) {
                bv.at(0, lwidth+1+i) = w.display[i];
            }
            bv.at(0, lwidth+1+w.display.size()) = RESET + "]";
            std::size_t self_width = w.display.size() + 2;
            std::size_t rstart = lwidth + self_width;
            auto [rwidth, rroot_span] = draw(w.right, bv.offset(2, rstart));
            if (rwidth) {
                std::size_t rroot = rroot_span.offset + (rroot_span.size / 2 - 1 + rroot_span.size%2);
                bv.at(0, rstart + rroot) = "╮";
                bv.at(1, rstart + rroot) = "│";
                for (std::size_t i = rstart; i < rstart + rroot; ++i) {
                    bv.at(0, i) = "─";
                }
            }
            return {lwidth + self_width + rwidth, Span(lwidth, self_width)};
        }

        std::pair<std::size_t, std::size_t> draw2(const Node* n,
                                                 StringBuff & sb,
                                                 std::size_t row,
                                                 std::size_t col)
        {
            if (n == nullptr) return {0, 0};
            Wrap w = wrap_map.at(n);
            auto [lwidth, lroot] = draw2(w.left, sb, row+1, col);
            if (lwidth) {
                lroot = (lroot + 1) / 2 - lroot%2;
                sb.at(row    , col + lroot) = "╭";
                for (std::size_t i = col + lroot + 1; i < col + lwidth; ++i) {
                    sb.at(row, i) = "─";
                }
            }
            sb.at(row, col+lwidth) = "[" + w.color;
            for (std::size_t i = 0; i < w.display.size(); ++i) {
                sb.at(row, col+lwidth+1+i) = w.display[i];
            }
            sb.at(row, col+lwidth+1+w.display.size()) = RESET + "]";
            std::size_t rstart = col+lwidth+2+w.display.size();
            auto [rwidth, rroot] = draw2(w.right, sb, row+1, rstart);
            if (rwidth) {
                rroot = rroot / 2 - 1 + rroot%2;
                sb.at(row    , rstart + rroot) = "╮";
                for (std::size_t i = rstart; i < rstart + rroot; ++i) {
                    sb.at(row, i) = "─";
                }
            }
            return {lwidth+2+w.display.size()+rwidth, lwidth + lwidth + 2 + w.display.size()};
        }
        std::pair<std::size_t, std::size_t> draw3(const Node* n,
                                                 StringBuff & sb,
                                                 std::size_t row,
                                                 std::size_t col)
        {
            if (n == nullptr) return {0, 0};
            Wrap w = wrap_map.at(n);
            auto [lwidth, lroot] = draw3(w.left, sb, row+2, col);
            if (lwidth) {
                lroot = (lroot + 1) / 2 - lroot%2;
                sb.at(row + 1, col + lroot) = "╭";
                sb.at(row+1, col+lwidth) = "╯";
                for (std::size_t i = col + lroot + 1; i < col + lwidth; ++i) {
                    sb.at(row+1, i) = "─";
                }
            }
            sb.at(row, col+lwidth) = " " + w.color;
            for (std::size_t i = 0; i < w.display.size(); ++i) {
                sb.at(row, col+lwidth+1+i) = w.display[i];
            }
            sb.at(row, col+lwidth+1+w.display.size()) = RESET + " ";
            std::size_t rstart = col+lwidth+2+w.display.size();
            auto [rwidth, rroot] = draw3(w.right, sb, row+2, rstart);
            if (rwidth) {
                rroot = rroot / 2 - 1 + rroot%2;
                sb.at(row + 1, rstart + rroot) = "╮";
                sb.at(row + 1, rstart - 1) = "╰";
                for (std::size_t i = rstart; i < rstart + rroot; ++i) {
                    sb.at(row+1, i) = "─";
                }
            }
            return {lwidth+2+w.display.size()+rwidth, lwidth + lwidth + 2 + w.display.size()};
        }
        public:
            WrappedTree(const Node* n): root(n) {
                wrap_map.emplace(nullptr, Wrap(nullptr, nullptr, nullptr, nullptr, "", "", 0, 0));
                wrap(n, nullptr);
            }
            StringBuff draw() {
                const Wrap &wroot = wrap_map.at(root);
                StringBuff sb(wroot.width, wroot.height * 2 - 1);
                draw(root, sb);
                return sb;
            }
            StringBuff draw2() {
                const Wrap &wroot = wrap_map.at(root);
                StringBuff sb(wroot.width, wroot.height);
                draw2(root, sb, 0, 0);
                return sb;
            }
            StringBuff draw3() {
                const Wrap &wroot = wrap_map.at(root);
                StringBuff sb(wroot.width, wroot.height * 2 - 1);
                draw3(root, sb, 0, 0);
                return sb;
            }
    };
    public:
        TreeHelper(const Node* root): root(root), current_wrap(root) {}
        StringBuff draw() {
            return current_wrap.draw();
        }
        StringBuff draw2() {
            return current_wrap.draw2();
        }
        StringBuff draw3() {
            return current_wrap.draw3();
        }
    private:
        const Node* root;
        WrappedTree current_wrap;
};
