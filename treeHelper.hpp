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
/// Terminal color escape codes
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string BLUE = "\033[34m";
const std::string GREY = "\033[90m";
const std::string RESET = "\033[0m";

const std::string DELETE_COLOR = GREY;
const std::string ADD_COLOR = GREEN;
const std::string CHANGE_COLOR = BLUE;
const std::string LOOP_COLOR = RED;

/**
 * String Buffer: a 2D array of "characters".
 * Each "character" displays as one character in a terminal, but internally
 * they are each stored in their own `std::string` instead of a `char`, 
 * because they may have attached terminal escape sequences for color, and
 * may also be unicode symbols wider than one byte.
 */
class StringBuff {
    public:
        std::size_t height;
        const std::size_t width;

        /// New buffer filled with `fill`
        StringBuff(std::size_t height, std::size_t width, std::string fill = " "): 
            height(height), width(width), buff(height*width, fill) {}

        /// Get mutable reference to specific position in buffer
        std::string & at(std::size_t row, std::size_t col) {
            return buff[row * width + col];
        }
        /// Get immutable reference to specific position in buffer
        const std::string & at(std::size_t row, std::size_t col) const {
            return buff[row * width + col];
        }
        /// Overwrite section of buffer, starting at [row, col], with contents of other buffer
        void paste(const StringBuff &other, std::size_t row = 0, std::size_t col = 0) {
            for (std::size_t r = 0; r < other.height; ++r) {
                for (std::size_t c = 0; c < other.width; ++c) {
                    this->at(row + r, col + c) = other.at(r, c);
                }
            }
        }
        /// Display the rectangular buffer
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

/**
 * Create a new string buffer containing the contents of two other buffers
 * pasted next to eachother, with variable spacing between.
 */
StringBuff horizontal_concat(const StringBuff &left,
                             const StringBuff &right,
                             std::size_t spacing = 0,
                             std::string fill = " ") {
    std::size_t height = std::max(left.height, right.height);
    std::size_t width = left.width + spacing + right.width;
    StringBuff dest(height, width, fill);
    dest.paste(left);
    dest.paste(right, 0, left.width + spacing);
    return dest;
}

/**
 * Buffer view: a non-owning "view" of a portion of a string buffer, 
 * starting at an offset.
 */
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
        std::size_t height() const {
            return buff.height - base_row;
        }
        std::size_t width() const {
            return buff.width - base_col;
        }
    private:
        StringBuff &buff;
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
template<std::size_t h = 1>
class SimpleLineDrawer {
    public:
        /// The number of rows 'total display height' increases per unit of tree height
        static const std::size_t row_inc = 1 + h;
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
                          std::string lcolor = "",
                          std::string rcolor = "") {
            // draw left arm
            if (lroot.size) {
                auto lroot_mid = lroot.offset + (lroot.size / 2);
                bv.at(0, lroot_mid) = lcolor + "╭";
                for (std::size_t i = 1; i <= h; ++i) {
                    bv.at(i, lroot_mid) = lcolor + "│" + RESET;
                }
                for (auto i = lroot_mid + 1; i < root.offset; ++i) {
                    bv.at(0, i) = "─";
                }
                // reset color for printing of root node
                bv.at(0, root.offset - 1) += RESET;
            }
            // draw right arm
            if (rroot.size) {
                auto rroot_mid = rroot.offset + ((rroot.size - 1) / 2);
                bv.at(0, rroot_mid) = "╮" + RESET;
                for (std::size_t i = 1; i <= h; ++i) {
                    bv.at(i, rroot_mid) = rcolor + "│" + RESET;
                }
                for (auto i = root.end(); i < rroot_mid; ++i) {
                    bv.at(0, i) = "─";
                }
                auto &s = bv.at(0, root.end());
                s = rcolor + s;
            }
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
        std::string color;
        // colors set for left & right arms of this node
        std::string lcolor;
        std::string rcolor;
        // width of display of subtree rooted at this node, in terminal chars
        std::size_t width;
        // node height of subtree
        std::size_t height;
        // number of terminal chars from the left this node will be printed
        std::size_t offset;
        // rank of node in tree
        std::size_t rank;
        
        // must always specify fields
        Wrap() = delete;

        // convenience initializer for all fields
        Wrap(
        const Node* n,
        const Node* parent,
        const Node* left,
        const Node* right,
        const bool left_loops,
        const bool right_loops,
        std::string display,
        std::string color,
        std::string lcolor,
        std::string rcolor,
        std::size_t width,
        std::size_t height,
        std::size_t offset,
        std::size_t rank
        ):
        n(n), parent(parent), left(left), right(right), 
        left_loops(left_loops), right_loops(right_loops),
        display(display /* boyfriend residue */), 
        color(color), lcolor(lcolor), rcolor(rcolor), width(width),
        height(height), offset(offset), rank(rank) {}
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
    std::optional<std::pair<std::size_t, std::size_t>> wrap(const Node* n, const Node* parent, std::size_t offset, std::size_t rank) {
        if (!visited.insert(n).second) return {};

        std::stringstream ss;
        ss << n->data;
        std::string display = ss.str();
        std::size_t self_width = display.size() + 2;

        std::size_t lwidth = 0;
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

        std::size_t rwidth = 0;
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

        std::size_t width = self_width + lwidth + rwidth;
        std::size_t height = 1 + std::max(lheight, rheight);

        wrap_map.emplace(n, Wrap(
            n, 
            parent,
            n->left,
            n->right,
            left_loops,
            right_loops,
            display,
            "",
            left_loops ? LOOP_COLOR : "",
            right_loops ? LOOP_COLOR : "",
            width,
            height,
            offset + lwidth,
            rank
        ));
        return {{width, height}};
    };

    template<class LinkDrawer>
    std::pair<std::size_t, Span> draw(const Node* n, BuffView bv) const {
        if (n == nullptr) return {0, Span(0, 0)};
        Wrap w = wrap_map.at(n);
        std::size_t self_width = w.display.size() + 2;

        std::size_t lwidth = 0;
        Span lroot(0, 0);
        if (!w.left_loops) {
            std::tie(lwidth, lroot) = draw<LinkDrawer>(w.left, bv.offset(LinkDrawer::row_inc, 0));
        }

        std::size_t rstart = lwidth + self_width;

        std::size_t rwidth = 0;
        Span rroot(0, 0);
        if (!w.right_loops) {
            std::tie(rwidth, rroot) = draw<LinkDrawer>(w.right, bv.offset(LinkDrawer::row_inc, rstart));
        }

        rroot.offset += rstart;
        Span self_span(lwidth, self_width);
        LinkDrawer::draw(bv, lroot, self_span, rroot, w.lcolor, w.rcolor);

        bv.at(0, lwidth) = w.lcolor + "[" + RESET + w.color;
        for (std::size_t i = 0; i < w.display.size(); ++i) {
            bv.at(0, lwidth+1+i) = w.display[i];
        }
        bv.at(0, lwidth+1+w.display.size()) = RESET + w.rcolor + "]" + RESET;

        return {lwidth + self_width + rwidth, self_span};
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

            LoopDrawInfo(
            std::size_t start_row,
            std::size_t start_col,
            std::size_t end_row,
            std::size_t end_col,
            std::size_t min_col,
            std::size_t max_col
            ):
            start_row(start_row),
            start_col(start_col),
            end_row(end_row),
            end_col(end_col),
            min_col(min_col),
            max_col(max_col) {}
        };
        auto compute_loop = [&](std::tuple<const Node *, Branch, const Node *> loop) {
            auto [src, side, dst] = loop;
            Wrap wsrc = wrap_map.at(src);
            Wrap wdst = wrap_map.at(dst);
            auto src_rank = wsrc.rank;
            auto dst_rank = wdst.rank;
            auto start_row = LinkDrawer::calc_height(src_rank);
            auto end_row = LinkDrawer::calc_height(dst_rank);
            auto start_col = wsrc.offset;
            auto end_col = wdst.offset + (wdst.display.size() + 2) / 2;
            if (side == RIGHT) {
                start_col += wsrc.display.size() + 1;
            }
            auto min_col = std::min(start_col, end_col);
            auto max_col = std::max(start_col, end_col);
            return LoopDrawInfo(
                    start_row,
                    start_col,
                    end_row,
                    end_col,
                    min_col,
                    max_col
                    );
        };
        std::vector<LoopDrawInfo> draw_infos;
        draw_infos.reserve(loops.size());
        for (auto loop : loops) {
            draw_infos.push_back(compute_loop(loop));
        }
        std::sort(draw_infos.begin(), draw_infos.end(), [](auto a, auto b) {
                return (a.max_col - a.min_col) < (b.max_col - b.min_col); });
        auto full_height = bv.height();
        auto base_horiz_row = full_height - loops.size();
        // draw all arrows first to avoid overwriting them
        for (auto draw_info : draw_infos) {
            bv.at(draw_info.end_row, draw_info.end_col) = LOOP_COLOR + "△" + RESET;
        }
        std::size_t prev_min = 0;
        for (auto draw_info : draw_infos) {
            auto [start_row, start_col, end_row, end_col, min_col, max_col] = draw_info;
            auto temp_horiz_row = base_horiz_row;
            auto max_row = std::max(start_row, end_row + 1);
            bool clear = true;
            while (
                    //temp_horiz_row > prev_min && 
                    temp_horiz_row >= max_row && clear
                    ) {
                --temp_horiz_row;
                for (auto c = min_col; c <= max_col; ++c) {
                    if (bv.at(temp_horiz_row, c) != " ") {
                        clear = false;
                        break;
                    }
                }
            }
            ++temp_horiz_row;
            const auto horiz_row = temp_horiz_row;
            prev_min = horiz_row;
            for (auto r = start_row; r < horiz_row; ++r) {
                auto & x = bv.at(r, start_col);
                if (x == " ") {
                    x = LOOP_COLOR + "│" + RESET;
                } else {
                    x = "│";
                }
            }
            for (auto c = min_col + 1; c < max_col; ++c) {
                auto & x = bv.at(horiz_row, c);
                if (x == " ") {
                    x = "─";
                }
            }
            bv.at(horiz_row, min_col) = LOOP_COLOR + "└";
            {
                auto & x = bv.at(horiz_row, max_col);
                if (x == " ") {
                    x = "┘" + RESET;
                } else {
                    x = "┘";
                }
            }
            for (auto r = horiz_row - 1; r > end_row; --r) {
                auto & x = bv.at(r, end_col);
                if (x == "┘" + RESET) {
                    x = "┤" + RESET;
                } else if (x == LOOP_COLOR + "└") {
                    x = LOOP_COLOR + "├";
                } else if (x == " ") {
                    x = LOOP_COLOR + "│" + RESET;
                } else if (x == "─") {
                    x = "│";
                }
            }
            base_horiz_row += 1;
        }
        std::cout << "nodes = " << full_height - loops.size() << std::endl;
        std::cout << "loops = " << prev_min << std::endl;
        return std::max(full_height - loops.size() - 1, prev_min + 1);
    }

    public:
        WrappedTree(const Node* n): root(n) {
            wrap_map.emplace(nullptr, Wrap(nullptr, nullptr, nullptr, nullptr, false, false, "", "", "", "", 0, 0, 0, 0));
            wrap(n, nullptr, 0, 1);
        }

        template<class LinkDrawer = SimpleLineDrawer<1>>
        StringBuff draw() const {
            const Wrap &wroot = wrap_map.at(root);
            StringBuff sb(LinkDrawer::calc_height(wroot.height) + loops.size() + 1, wroot.width);
            draw<LinkDrawer>(root, sb);
            auto height = draw_loops<LinkDrawer>(sb);
            sb.height = height;
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
