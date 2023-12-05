#pragma once
#include<string>
#include<sstream>
#include<tuple>
#include<algorithm>
#include<vector>
#include<unordered_map>
#include<unordered_set>
#include<utility>
#include<iostream>
#include<optional>
#include<array>
/// Terminal color escape codes
enum ForegroundColor: char {
    DEFAULT = 0,
    RED,
    GREEN,
    BLUE,
    GREY
};

std::ostream & operator<<(std::ostream & os, ForegroundColor c) {
    switch (c) {
        case DEFAULT: os << "\033[0m"; break;
        case RED: os << "\033[31m"; break;
        case GREEN: os << "\033[32m"; break;
        case BLUE: os << "\033[34m"; break;
        case GREY: os << "\033[90m"; break;
        // default: os.setstate(std::ios_base::failbit);
    };
    return os;
}

const std::size_t HSPACING = 1;
const std::size_t VSPACING = 0;

const std::string LEFT_SURROUND = "(";
const std::string RIGHT_SURROUND = ")";

const ForegroundColor DELETE_COLOR = GREY;
const ForegroundColor ADD_COLOR = GREEN;
const ForegroundColor CHANGE_COLOR = BLUE;
const ForegroundColor LOOP_COLOR = RED;

template<std::size_t WIDTH>
struct TinyStr {
    std::array<char, WIDTH> chars;

    TinyStr(): chars{'\0'} {}

    TinyStr(char c): chars{'\0'} {
        static_assert(WIDTH >= 1);
        chars[0] = c;
    }

    template<std::size_t N>
    TinyStr(const char (&s)[N]): chars{'\0'} {
        static_assert(N <= (WIDTH+1));
        for (std::size_t i = 0; i < (N-1); ++i) {
            chars[i] = s[i];
        }
    }

    template<std::size_t N>
    bool operator==(const char (&s)[N]) {
        static_assert(N <= (WIDTH+1));
        bool matches = true;
        for (std::size_t i = 0; i < (N-1); ++i) {
            matches = matches && (chars[i] == s[i]);
        }
        return matches;
    }

    friend std::ostream & operator<<(std::ostream & os, const TinyStr mt) {
        for (std::size_t i = 0; i < WIDTH; ++i) {
            if (mt.chars[i] != '\0') os << mt.chars[i];
        }
        return os;
    }
};

struct TermCell {
    ForegroundColor color;
    TinyStr<3> c;
    TermCell(): TermCell(DEFAULT, ' ') {}
    TermCell(ForegroundColor color, char c): color(color), c(c) {}
    template<std::size_t N>
    TermCell(ForegroundColor color, const char (&c)[N]): color(color), c(c) {}
};

class TermBuff {
    public:
        std::size_t height;
        const std::size_t width;

        /// New buffer filled with `fill`
        TermBuff(std::size_t height, std::size_t width): 
            height(height), width(width), buff(height*width, TermCell()) {}

        /// Get mutable reference to specific position in buffer
        TermCell & at(std::size_t row, std::size_t col) {
            return buff[row * width + col];
        }
        /// Get immutable reference to specific position in buffer
        const TermCell & at(std::size_t row, std::size_t col) const {
            return buff[row * width + col];
        }
        /// Overwrite section of buffer, starting at [row, col], with contents of other buffer
        void paste(const TermBuff &other, std::size_t row = 0, std::size_t col = 0) {
            for (std::size_t r = 0; r < other.height; ++r) {
                for (std::size_t c = 0; c < other.width; ++c) {
                    this->at(row + r, col + c) = other.at(r, c);
                }
            }
        }
        /// Display the rectangular buffer
        friend std::ostream & operator<<(std::ostream &os, const TermBuff &sb) {
            size_t buff_size = sb.height * sb.width;
            for (std::size_t i = 0; i < buff_size; i += sb.width) {
                ForegroundColor color = DEFAULT;
                for (std::size_t j = 0; j < sb.width; ++j) {
                    const auto & x = sb.buff[i+j];
                    if (color != x.color) {
                        color = x.color;
                        os << x.color;
                    }
                    os << x.c;
                }
                os << DEFAULT << std::endl;
            }
            return os;
        }
    private:
        std::vector<TermCell> buff;
};

/**
 * Create a new terminal buffer containing the contents of two other buffers
 * pasted next to eachother, with variable spacing between.
 */
TermBuff horizontal_concat(const TermBuff &left,
                             const TermBuff &right,
                             std::size_t spacing = 0) {
    std::size_t height = std::max(left.height, right.height);
    std::size_t width = left.width + spacing + right.width;
    TermBuff dest(height, width);
    dest.paste(left);
    dest.paste(right, 0, left.width + spacing);
    return dest;
}

/**
 * Buffer view: a non-owning "view" of a portion of a terminal buffer, 
 * starting at an offset.
 */
class BuffView {
    public:
        BuffView(TermBuff &buff, std::size_t base_row = 0, std::size_t base_col = 0):
            buff(buff), base_row(base_row), base_col(base_col) {}
        BuffView offset(std::size_t rows, std::size_t cols) {
            return BuffView(buff, base_row + rows, base_col + cols);
        }
        TermCell & at(std::size_t row, std::size_t col) {
            return buff.at(base_row + row, base_col + col);
        }
        std::size_t height() const {
            return buff.height - base_row;
        }
        std::size_t width() const {
            return buff.width - base_col;
        }
    private:
        TermBuff &buff;
        std::size_t base_row;
        std::size_t base_col;
};

/**
 * A span in a one dimensional structure: an offset and a size (length), where
 * the end is `offset + size`.
 */
struct Span {
    std::size_t offset;
    std::size_t size;
    Span(std::size_t offset, std::size_t size): offset(offset), size(size) {}
    std::size_t end() {
        return offset + size;
    }
};

/**
 * Draws the lines of the arms of a binary tree. `h` is the extra lines of
 * height drawn.
 */
class SimpleLineDrawer {
    public:
        /// The number of rows 'total display height' increases per unit of tree height
        static const std::size_t row_inc = 1 + VSPACING;
        /// Calculate total display height from height of tree.
        static std::size_t calc_height(std::size_t nodes) {
            return 1 + row_inc * (nodes - 1);
        }
        /** 
         * Draw arms of binary tree into provided buffer view, based on the spans of
         * the root of the left subtree, the root, and the root of the right subtree.
         *
         * lroot:          ______
         * root:                 _______
         * rroot:                       _______
         * lroot_mid:         ↓
         * root.offset:          ↓
         * root.end():                  ↓
         * rroot_mid:                      ↓
         *                    ╭──[12000]───╮
         * h = 1:             │            │
         *                 [1000]       [13000]
         *
         * The arithmetic done to find the midpoints is to achieve the following properties:
         *   - Odd length nodes have the arm connect above the middle character
         *   - Of the two "middle" characters in even length nodes, the arm connects above the one
         *     that results in the shorter arm length.
         * 
         * lroot.size:   3     4     5       6       7         8    
         * lroot_mid:    1     2     2       3       3         4    
         *               ╭─    ╭─    ╭──     ╭──     ╭───      ╭───
         *              [1]  [12]  [123]  [1234]  [12345]  [123456]
         *
         * rroot.size:   3    4      5      6        7        8    
         * rroot_mid:    1    1      2      2        3        3    
         *               ╮   ─╮    ──╮    ──╮     ───╮     ───╮   
         *              [1]  [12]  [123]  [1234]  [12345]  [123456]
         */
        static void draw(BuffView bv,
                          Span lroot,
                          Span root,
                          Span rroot,
                          ForegroundColor lcolor,
                          ForegroundColor rcolor) {
            // draw left arm
            if (lroot.size) {
                auto lroot_mid = lroot.offset + (lroot.size / 2);
                bv.at(0, lroot_mid) = {lcolor, "╭"};
                for (std::size_t i = 1; i <= VSPACING; ++i) {
                    bv.at(i, lroot_mid) = {lcolor, "│"};
                }
                for (auto i = lroot_mid + 1; i < root.offset; ++i) {
                    bv.at(0, i) = {lcolor, "─"};
                }
            }
            // draw right arm
            if (rroot.size) {
                auto rroot_mid = rroot.offset + ((rroot.size - 1) / 2);
                bv.at(0, rroot_mid) = {rcolor, "╮"};
                for (std::size_t i = 1; i <= VSPACING; ++i) {
                    bv.at(i, rroot_mid) = {rcolor, "│"};
                }
                for (auto i = root.end(); i < rroot_mid; ++i) {
                    bv.at(0, i) = {rcolor, "─"};
                }
            }
        }
};

/**
 * Width of some number of in-order nodes in a tree
 */
struct TreeWidth {
    std::size_t node_count;
    std::size_t displays_sum;

    TreeWidth operator+(const TreeWidth & other) const {
        return {node_count + other.node_count, displays_sum + other.displays_sum};
    }

    std::size_t display_width(std::size_t spacing) const {
        return node_count ? LEFT_SURROUND.size() + displays_sum + spacing * (node_count - 1) + RIGHT_SURROUND.size() : 0;
    }

    std::size_t content_offset(std::size_t spacing) const {
        return LEFT_SURROUND.size() + displays_sum + spacing * node_count;
    }

    std::size_t surround_offset(std::size_t spacing) const {
        return node_count ? LEFT_SURROUND.size() + displays_sum + spacing * node_count - RIGHT_SURROUND.size() : 0;
    }
};

/**
 * Captured snapshot of a binary tree, with pretty display functionality.
 * It is generic over the Node type. The Node is expected to have accessible data members:
 *     Node * left;
 *     Node * right;
 *     Data data;
 * where `Data` implements an `operator<<(std::ostream, const Data &)`
 */
template<typename Node>
class WrappedTree {
    using Data = decltype(Node::data);
    /**
     * Snapshot of single node state at point in time.
     */
    struct Wrap {
        // recorded pointers from original tree
        const Node* n;
        const Node* parent;
        const Node* left;
        const Node* right;

        // whether either of left or right point back up the tree
        const bool left_loops;
        const bool right_loops;

        // content of data written to a stringstream
        std::string display;
        // color set for this node
        ForegroundColor color;
        // colors set for left & right arms of this node
        ForegroundColor lcolor;
        ForegroundColor rcolor;
        // width of display of subtree rooted at this node, in terminal chars
        TreeWidth width;
        // node height of subtree
        std::size_t height;
        // offset from the beginning of the tree
        TreeWidth offset;
        // rank of node in tree
        std::size_t rank;
        
        // must always specify fields
        Wrap() = delete;
    };
    // store of wrapped nodes for each original node pointer
    std::unordered_map<const Node*, Wrap> wrap_map;
    // visited nodes
    std::unordered_set<const Node*> visited;

    enum Branch {
        LEFT,
        RIGHT
    };
    // loops in the tree: src, branch, dst
    std::vector<std::tuple<const Node *, Branch, const Node *>> loops;
    // original tree root
    const Node* root;
    // record a node in the tree
    std::optional<std::pair<TreeWidth, std::size_t>> 
    wrap(const Node* n, const Node* parent, TreeWidth offset, std::size_t rank) {
        if (!visited.insert(n).second) return {};

        std::stringstream ss;
        ss << n->data;
        std::string display = ss.str();
        TreeWidth self_width {1, display.size()};

        TreeWidth lwidth {0, 0};
        std::size_t lheight = 0;
        bool left_loops = false;
        if (n->left) {
            auto mlw = wrap(n->left, n, offset, rank + 1);
            if (mlw.has_value()) {
                auto lw = mlw.value();
                lwidth = lw.first;
                lheight = lw.second;
            } else {
                left_loops = true;
                loops.emplace_back(n, LEFT, n->left);
            }
        }

        TreeWidth rwidth {0, 0};
        std::size_t rheight = 0;
        bool right_loops = false;
        if (n->right) {
            auto mrw = wrap(n->right, n, offset + lwidth + self_width, rank + 1);
            if (mrw.has_value()) {
                auto rw = mrw.value();
                rwidth = rw.first;
                rheight = rw.second;
            } else {
                right_loops = true;
                loops.emplace_back(n, RIGHT, n->right);
            }
        }

        TreeWidth subtree_width = self_width + lwidth + rwidth;
        std::size_t height = 1 + std::max(lheight, rheight);

        wrap_map.emplace(n, Wrap{
            n, 
            parent,
            n->left,
            n->right,
            left_loops,
            right_loops,
            display,
            DEFAULT,
            left_loops ? LOOP_COLOR : DEFAULT,
            right_loops ? LOOP_COLOR : DEFAULT,
            subtree_width,
            height,
            offset + lwidth,
            rank
            });
        return {{subtree_width, height}};
    };

    template<class LinkDrawer>
    std::pair<TreeWidth, Span> draw(const Node* n, BuffView bv) const {
        if (n == nullptr) return {{0, 0}, Span(0, 0)};
        Wrap w = wrap_map.at(n);

        TreeWidth lwidth {0, 0};
        Span lroot(0, 0);
        if (!w.left_loops) {
            std::tie(lwidth, lroot) = draw<LinkDrawer>(w.left, bv.offset(LinkDrawer::row_inc, 0));
        }

        std::size_t rstart = (lwidth + TreeWidth {1, w.display.size()}).surround_offset(HSPACING);

        TreeWidth rwidth {0, 0};
        Span rroot(0, 0);
        if (!w.right_loops) {
            std::tie(rwidth, rroot) = draw<LinkDrawer>(w.right, bv.offset(LinkDrawer::row_inc, rstart));
        }

        std::size_t left_surround_offset = lwidth.surround_offset(HSPACING);

        rroot.offset += rstart;
        Span self_span(left_surround_offset, LEFT_SURROUND.size() + w.display.size() + RIGHT_SURROUND.size());
        LinkDrawer::draw(bv, lroot, self_span, rroot, w.lcolor, w.rcolor);

        for (std::size_t i = 0; i < LEFT_SURROUND.size(); ++i) {
            bv.at(0, left_surround_offset+i) = {w.lcolor, LEFT_SURROUND[i]};
        }
        std::size_t content_offset = left_surround_offset + LEFT_SURROUND.size();
        for (std::size_t i = 0; i < w.display.size(); ++i) {
            bv.at(0, content_offset+i) = {w.color, w.display[i]};
        }
        std::size_t right_surround_offset = content_offset + w.display.size();
        for (std::size_t i = 0; i < RIGHT_SURROUND.size(); ++i) {
            bv.at(0, right_surround_offset+i) = {w.rcolor, RIGHT_SURROUND[i]};
        }

        return {w.width, self_span};
    }

    template<class LinkDrawer>
    std::size_t draw_loops(BuffView bv) const {
        struct LoopDrawInfo {
            std::size_t start_row;
            std::size_t start_col;
            std::size_t end_row;
            std::size_t end_col;
            std::size_t min_col;
            std::size_t max_col;
            std::size_t bottom_row;
        };
        auto compute_loop = [&](std::tuple<const Node *, Branch, const Node *> loop) {
            auto [src, side, dst] = loop;
            Wrap wsrc = wrap_map.at(src);
            Wrap wdst = wrap_map.at(dst);
            auto src_rank = wsrc.rank;
            auto dst_rank = wdst.rank;
            auto start_row = LinkDrawer::calc_height(src_rank);
            auto end_row = LinkDrawer::calc_height(dst_rank);
            std::size_t start_col;
            if (side == LEFT) {
                start_col = wsrc.offset.content_offset(HSPACING) - 1;
            } else {
                start_col = wsrc.offset.content_offset(HSPACING) + wsrc.display.size();
            }
            auto end_col = wdst.offset.content_offset(HSPACING) + wdst.display.size() / 2;
            auto min_col = std::min(start_col, end_col);
            auto max_col = std::max(start_col, end_col);
            return LoopDrawInfo{
                start_row,
                start_col,
                end_row,
                end_col,
                min_col,
                max_col,
                0
            };
        };
        std::vector<LoopDrawInfo> draw_infos;
        draw_infos.reserve(loops.size());
        for (auto loop : loops) {
            draw_infos.push_back(compute_loop(loop));
        }
        std::sort(draw_infos.begin(), draw_infos.end(), [](auto a, auto b) {
                return (a.max_col - a.min_col) < (b.max_col - b.min_col); });
        auto full_height = bv.height();
        // draw all arrows first to avoid overwriting them
        for (auto draw_info : draw_infos) {
            bv.at(draw_info.end_row, draw_info.end_col) = {LOOP_COLOR, "△"};
        }
        std::size_t lowest_loop = 0;
        for (auto & draw_info : draw_infos) {
            auto & [start_row, start_col, end_row, end_col, min_col, max_col, bottom_row] = draw_info;
            auto max_row = std::max(start_row, end_row + 1);
            auto first_clear_row = max_row;
            while (true) {
                bool clear = true;
                for (auto c = min_col; c <= max_col; ++c) {
                    if (!(bv.at(first_clear_row, c).c == " ")) {
                        clear = false;
                        break;
                    }
                }
                if (clear) {
                    break;
                } else {
                    ++first_clear_row;
                }
            }
            bottom_row = first_clear_row;
            bv.at(bottom_row, min_col) = {LOOP_COLOR, "└"};
            bv.at(bottom_row, max_col) = {LOOP_COLOR, "┘"};
            for (auto c = min_col + 1; c < max_col; ++c) {
                bv.at(first_clear_row, c) = {LOOP_COLOR, "─"};
            }
            lowest_loop = std::max(lowest_loop, first_clear_row);
        }
        for (auto draw_info : draw_infos) {
            auto [start_row, start_col, end_row, end_col, min_col, max_col, bottom_row] = draw_info;
            for (auto r = start_row; r < bottom_row; ++r) {
                bv.at(r, start_col) = {LOOP_COLOR, "│"};
            }
            for (auto r = bottom_row - 1; r > end_row; --r) {
                auto & x = bv.at(r, end_col);
                if (x.c == "┘") {
                    x = {LOOP_COLOR, "┤"};
                } else if (x.c == "└") {
                    x = {LOOP_COLOR, "├"};
                } else if (x.c == " ") {
                    x = {LOOP_COLOR, "│"};
                } else if (x.c == "─") {
                    x = {LOOP_COLOR, "│"};
                }
            }
        }
        return std::max(full_height - loops.size() - 1, lowest_loop + 1);
    }

    public:
        WrappedTree(const Node* n): root(n) {
            wrap_map.emplace(nullptr, Wrap{nullptr, nullptr, nullptr, nullptr, false, false, "", DEFAULT, DEFAULT, DEFAULT, {0, 0}, 0, {0, 0}, 0});
            wrap(n, nullptr, {0, 0}, 1);
        }

        template<class LinkDrawer = SimpleLineDrawer>
        TermBuff draw() const {
            const Wrap &wroot = wrap_map.at(root);
            TermBuff sb(LinkDrawer::calc_height(wroot.height) + loops.size() + 1, wroot.width.display_width(HSPACING));
            draw<LinkDrawer>(root, sb);
            auto height = draw_loops<LinkDrawer>(sb);
            sb.height = height;
            return sb;
        }
        void color_node(const Node* n, ForegroundColor color) {
            wrap_map.at(n).color = color;
        }
        std::pair<WrappedTree, WrappedTree> compare_to(const WrappedTree &other) const {
            WrappedTree oldt(*this);
            WrappedTree newt(other);
            for (auto & p : oldt.wrap_map) {
                const Node* n = p.first;
                Wrap &wr = p.second;
                const auto found = newt.wrap_map.find(n);
                if (found == newt.wrap_map.end()) {
                    wr.color = DELETE_COLOR;
                    wr.lcolor = DELETE_COLOR;
                    wr.rcolor = DELETE_COLOR;
                }
            }
            for (auto & p : newt.wrap_map) {
                const Node* n = p.first;
                Wrap &wr = p.second;
                const auto found = oldt.wrap_map.find(n);
                if (found == oldt.wrap_map.end()) {
                    wr.color = ADD_COLOR;
                } else {
                    const auto & oldwr = found->second;
                    if (oldwr.left != wr.left) {
                        wr.lcolor = CHANGE_COLOR;
                    }
                    if (oldwr.right != wr.right) {
                        wr.rcolor = CHANGE_COLOR;
                    }
                }
            }
            return {oldt, newt};
        }
        friend std::ostream & operator<<(std::ostream &os, const WrappedTree<Node> &wt) {
            return os << wt.draw();
        }
};
