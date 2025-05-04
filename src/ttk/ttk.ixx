module;
#include "../global.hpp"
export module cpptkinter:ttk;
export import :ttk.cnfs;
import :utility;
import :cpptkinter.cnfs;
import :cpptkinter.misc;
import :cpptkinter.widget.base;
import std;
import hhh;

export namespace cpptkinter::detail
{
	/// @brief std::string or std::size_t
	template<typename T>
	concept tab_id_arg = utility::union_arg<T, std::size_t, std::string>;
	constexpr auto to_tab_id_arg = utility::to_union_arg<std::size_t, std::string>;

	/// @brief std::string or long long
	template<typename T>
	concept item_arg = utility::union_arg<T, long long, std::string>;
	constexpr auto to_item_arg = utility::to_union_arg<long long, std::string>;
	template<typename R>
	concept range_of_item_arg = utility::range_of_union_arg<R, long long, std::string>;

	/// @brief std::string or long long
	template<typename T>
	concept column_arg = utility::union_arg<T, long long, std::string>;
	constexpr auto to_column_arg = utility::to_union_arg<long long, std::string>;

	struct Notebook_tab_return
	{
		std::string state;
		std::string sticky;
		std::vector<long long> padding;
		std::string text;
		std::string image;
		std::string compound;
		std::string underline;
	};

	struct Treeview_column_return
	{
		std::string id;
		std::string anchor;
		long long minwidth;
		bool separator;
		bool stretch;
		long long width;
	};

	struct Treeview_heading_return
	{
		/// The text to display in the column heading
		std::string text;
		/// Specifies an image to display to the right of the column heading. 
		std::string image;
		/// Specifies how the heading text should be aligned. One of the standard Tk anchor values. 
		std::string anchor;
		/// A script to evaluate when the heading label is pressed. 
		std::string command;
		std::string state;
	};

	struct Treeview_item_return
	{
		long long height;
		bool hidden;
		std::string image;
		std::string imageanchor;
		bool open;
		std::vector<std::string> tags;
		std::string text;
		std::vector<std::variant<long long, std::string>> values;
	};
}

export namespace cpptkinter::ttk
{
	/// @brief Base class for Tk themed widgets.
	struct Widget : cpptkinter::Widget
	{
		using cpptkinter::Widget::Widget;

		/// @brief Returns the name of the element at position x, y, or the empty string if the point does not lie within any element.
		/// 
		/// x and y are pixel coordinates relative to the widget.
		std::string identify(long long x, long long y)
		{
			return this->tk->call<std::string>(this->_w, "identify", x, y);
		}

		/// @brief Test the widget's state.
		/// 
		/// @returns true if the widget state matches statespec and false otherwise.
		bool instate(const std::vector<std::string>& statespec)
		{
			return this->tk->call<bool>(this->_w, "instate", hhh::misc::join_strings(statespec, " "));
		}

		/// @brief Inquire widget state.
		std::vector<std::string> state()
		{
			return this->tk->call<std::vector<std::string>>(this->_w, "state");
		}
		/// @brief Modify widget state.
		/// 
		/// Widget state is returned if statespec is None, otherwise it is set according to the statespec flags
		/// and then a new state spec is returned indicating which flags were changed.
		std::vector<std::string> state(const std::vector<std::string>& statespec)
		{
			return this->tk->call<std::vector<std::string>>(this->_w, "state", statespec);
		}
	};

	/// @brief Ttk Notebook widget manages a collection of windows and displays a single one at a time.
	/// Each child window is associated with a tab, which the user may select to change the currently - displayed window.
	struct Notebook : Widget
	{
		/// @brief Construct a menubutton widget.
		CONSTRUCTORS_AND_ASSIGNMENT(Notebook, cnfs::Notebook, "ttk::notebook", Widget);

		/// @brief Adds a new tab to the notebook.
		/// 
		/// If window is currently managed by the notebook but hidden, it is restored to its previous position.
		void add(const cnfs::Notebook_add& cnf)
		{
			this->tk->call(this->_w, "add", cnf.child, this->_options(cnf, { "child" }));
		}

		/// @brief Removes the tab specified by tab_id, unmaps and unmanages the associated window.
		void forget(detail::tab_id_arg auto&& tab_id)
		{
			this->tk->call(this->_w, "forget", detail::to_tab_id_arg(tab_id));
		}

		/// @brief Hides the tab specified by tab_id.
		/// 
		/// The tab will not be displayed, but the associated window remains managed by the notebook and its configuration remembered.
		/// Hidden tabs may be restored with the add command.
		void hide(detail::tab_id_arg auto&& tab_id)
		{
			this->tk->call(this->_w, "hide", detail::to_tab_id_arg(tab_id));
		}

		/// @brief Returns the name of the tab element at position x, y, or the empty string if none.
		std::string identify(long long x, long long y)
		{
			return this->tk->call<std::string>(this->_w, "identify", x, y);
		}

		/// @brief Returns the numeric index of the tab specified by tab_id, or the total number of tabs if tab_id is the string "end".
		std::size_t index(detail::tab_id_arg auto&& tab_id)
		{
			return this->tk->call<std::size_t>(this->_w, "index", detail::to_tab_id_arg(tab_id));
		}

		/// @brief Inserts a pane at the specified position.
		/// 
		/// pos is either the string end, an integer index, or the name of a managed child.
		/// If child is already managed by the notebook, moves it to the specified position.
		void insert(const cnfs::Notebook_insert& cnf)
		{
			this->tk->call(this->_w, "insert", cnf.pos, cnf.child, this->_options(cnf, { "pos", "child" }));
		}

		/// @brief Returns the widget name of the currently selected pane.
		std::string select()
		{
			return this->tk->call<std::string>(this->_w, "select");
		}
		/// @brief Selects the specified tab.
		/// 
		/// The associated child window will be displayed, and the previously - selected window(if different) is unmapped.
		void select(detail::tab_id_arg auto&& tab_id)
		{
			this->tk->call(this->_w, "select", detail::to_tab_id_arg(tab_id));
		}

		/// @brief Query the options of the specific tab_id.
		detail::Notebook_tab_return tab(detail::tab_id_arg auto&& tab_id)
		{
			using V = std::variant<long long, std::string, std::vector<long long>>;
			using M = std::map<std::string, V>;

			auto vec = this->tk->call<std::vector<V>>(this->_w, "tab", detail::to_tab_id_arg(tab_id));
			auto map = detail::vector_to_map<V>(std::move(vec));

			return detail::_splitdict_to_aggregate<detail::Notebook_tab_return>(std::move(map));
		}
		/// @brief Modify the options of the specific tab_id.
		void tab(const cnfs::Notebook_tab& cnf)
		{
			this->tk->call(this->_w, "tab", cnf.tab_id, this->_options(cnf, { "tab_id" }));
		}

		/// @brief Returns a list of windows managed by the notebook.
		std::vector<std::string> tabs()
		{
			return this->tk->call<std::vector<std::string>>(this->_w, "tabs");
		}

		/// @brief Enable keyboard traversal for a toplevel window containing this notebook.
		/// 
		/// This will extend the bindings for the toplevel window containing this notebook as follows:
		/// 
		/// - Control-Tab: selects the tab following the currently selected one
		/// - Shift-Control-Tab: selects the tab preceding the currently selected one
		/// - Alt-K: where K is the mnemonic (underlined) character of any tab, will select that tab.
		/// 
		/// Multiple notebooks in a single toplevel may be enabled for traversal, including nested notebooks.
		/// However, notebook traversal only works properly if all panes are direct children of the notebook.
		void enable_traversal()
		{
			this->tk->call("ttk::notebook::enableTraversal", this->_w);
		}
	};

	/// @brief Ttk Treeview widget displays a hierarchical collection of items.
	/// 
	/// Each item has a textual label, an optional image, and an optional list of data values.
	/// The data values are displayed in successive columns after the tree label.
	struct Treeview : Widget, XView<Treeview>, YView<Treeview>
	{
		/// @brief Construct a menubutton widget.
		CONSTRUCTORS_AND_ASSIGNMENT(Treeview, cnfs::Treeview, "ttk::treeview", Widget);

		/// @brief Returns the bounding box (relative to the treeview widget's window) of the specified item in the form x y width height.
		std::array<long long, 4> bbox(detail::item_arg auto&& item)
		{
			return this->tk->call<std::array<long long, 4>>(this->_w, "bbox", detail::to_item_arg(item));
		}
		/// @brief Returns the bounding box (relative to the treeview widget's window) of the specified item and column in the form x y width height.
		std::array<long long, 4> bbox(detail::item_arg auto&& item, detail::column_arg auto&& column)
		{
			return this->tk->call<std::array<long long, 4>>(this->_w, "bbox", detail::to_item_arg(item), detail::to_column_arg(column));
		}

		/// @brief Returns a tuple of children belonging to item.
		/// 
		/// If item is not specified, returns root children.
		std::vector<std::string> get_children(detail::item_arg auto&& item)
		{
			return this->tk->call<std::vector<std::string>>(this->_w, "children", detail::to_item_arg(item));
		}
		/// @copydoc get_children
		std::vector<std::string> get_children(detail::item_arg auto&& item = "")
		{
			return this->get_children("");
		}

		/// @brief Replaces item's child list with newchildren.
		/// 
		/// Children present in item that are not present in newchildren are detached from tree.
		/// No items in newchildren may be an ancestor of item.
		void set_children(detail::item_arg auto&& item, detail::item_arg auto&&...newchildren)
		{
			this->tk->call(this->_w, "children", detail::to_item_arg(item), detail::to_item_arg(newchildren)...);
		}
		/// @brief Replaces item's child list with newchildren.
		/// 
		/// Children present in item that are not present in newchildren are detached from tree.
		/// No items in newchildren may be an ancestor of item.
		void set_children(detail::item_arg auto&& item, detail::range_of_item_arg auto&& newchildren)
		{
			this->tk->call(this->_w, "children", detail::to_item_arg(item), newchildren | std::views::transform(detail::to_item_arg));
		}

		/// @brief Query the options for the specified column.
		detail::Treeview_column_return column(detail::column_arg auto&& column)
		{
			using V = std::variant<bool, long long, std::string>;
			using M = std::map<std::string, V>;

			auto vec = this->tk->call<std::vector<V>>(this->_w, "column", detail::to_column_arg(column));
			auto map = detail::vector_to_map<V>(std::move(vec));

			return detail::_splitdict_to_aggregate<detail::Treeview_column_return>(std::move(map));
		}
		/// @brief Modify the options for the specified column.
		template<cnfs::is_cnf CNF = cnfs::Treeview_column>
		void column(CNF&& cnf)
		{
			this->tk->call(this->_w, "column", cnf.column, this->_options(std::forward<CNF>(cnf), { "column" }));
		}

		/// @brief Delete all specified items and all their descendants. The root item may not be deleted.
		void delete_(detail::item_arg auto&&...items)
		{
			this->tk->call(this->_w, "delete", detail::to_item_arg(items)...);
		}
		/// @copydoc delete_
		void delete_(detail::range_of_item_arg auto&& items)
		{
			this->tk->call(this->_w, "delete", items | std::views::transform(detail::to_item_arg));
		}

		/// @brief 
		void detach();

		/// @brief 
		void exists();

		/// @brief 
		void focus();

		/// @brief Query the heading options for the specified column.
		/// 
		/// To query the tree column heading, call this with column = "#0"
		detail::Treeview_heading_return heading(detail::column_arg auto&& column)
		{
			using V = std::variant<std::string, std::vector<long long>>; // image can be an empty tcl list for some reason, therefor we need vector
			using M = std::map<std::string, V>;

			auto vec = this->tk->call<std::vector<V>>(this->_w, "heading", detail::to_column_arg(column));
			auto map = detail::vector_to_map<V>(std::move(vec));

			auto it = map.find("-image");
			if (it != map.end())
				it->second = std::string();

			return detail::_splitdict_to_aggregate<detail::Treeview_heading_return>(std::move(map));
		}
		/// @brief Modify the heading options for the specified column.
		/// 
		/// To query the tree column heading, call this with column = "#0"
		template<cnfs::is_cnf CNF = cnfs::Treeview_heading>
		void heading(CNF&& cnf)
		{
			this->tk->call(this->_w, "heading", cnf.column, this->_options(std::forward<CNF>(cnf), { "column" }));
		}

		/// @brief Returns a description of the specified component under the point given by x and y, or the empty string if no such component is present at that position.
		std::string identify(const std::string& component, long long x, long long y)
		{
			return this->tk->call<std::string>(this->_w, "identify", component, x, y);
		}

		/// @brief Returns the item ID of the item at position y.
		std::string identify_row(long long y)
		{
			return this->identify("row", 0, y);
		}

		/// @brief Returns the data column identifier of the cell at position x.
		/// 
		/// The tree column has ID #0.
		std::string identify_column(long long x)
		{
			return this->identify("column", x, 0);
		}

		/// @brief Returns one of:
		/// 
		/// - heading: Tree heading area.
		/// -separator : Space between two columns headings;
		/// - tree: The tree area.
		/// - cell : A data cell.
		std::string identify_region(long long x, long long y)
		{
			return this->identify("region", x, y);
		}

		/// @brief Returns the element at position x, y.
		std::string identify_element(long long x, long long y)
		{
			return this->identify("element", x, y);
		}

		/// @brief 
		void index();

		/// @brief Creates a new item and return the item identifier of the newly created item.
		/// 
		/// parent is the item ID of the parent item, or the empty string to create a new top-level item.
		/// index is an integer, or the value end, specifying where in the list of parent's children to insert the new item.
		/// If index is less than or equal to zero, the new node is inserted at the beginning.
		/// If index is greater than or equal to the current number of children, it is inserted at the end.
		/// If iid is specified, it is used as the item identifier, iid must not already exist in the tree.
		/// Otherwise, a new unique identifier is generated.
		template<cnfs::is_cnf CNF = cnfs::Treeview_insert>
		std::string insert(CNF&& cnf)
		{
			return this->tk->call<std::string>(this->_w, "insert", cnf.parent, cnf.index, this->_options(std::forward<CNF>(cnf), { "parent", "index" }));
		}

		/// @brief Query the options for the specified item.
		detail::Treeview_item_return item(detail::item_arg auto&& item)
		{
			using V = std::variant<bool, long long, std::string, std::vector<std::variant<long long, std::string>>, std::vector<std::string>>;
			using M = std::map<std::string, V>;
			auto vec = this->tk->call<std::vector<V>>(this->_w, "item", detail::to_item_arg(item));
			//this->tk->call<M>(this->_w, "item", detail::to_item_arg(item));
			auto map = detail::vector_to_map<V>(std::move(vec));

			auto it = map.find("-hidden");
			if (it != map.end())
				it->second = bool(std::get<long long>(it->second));

			it = map.find("-open");
			if (it != map.end())
				it->second = bool(std::get<long long>(it->second));

			it = map.find("-tags");
			if (it != map.end())
				if (std::holds_alternative<std::string>(it->second) && std::get<std::string>(it->second).empty())
					it->second = std::vector<std::string>();

			return detail::_splitdict_to_aggregate<detail::Treeview_item_return>(std::move(map));
		}
		/// @brief Modify the options for the specified item.
		/// 
		/// If no options are given, a dict with options/values for the item is returned.
		/// If option is specified then the value for that option is returned.
		/// Otherwise, sets the options to the corresponding values as given by kw.
		void item();

		/// @brief 
		void move();

		/// @brief 
		void reattach();

		/// @brief 
		void next();

		/// @brief 
		void parent();

		/// @brief 
		void prev();

		/// @brief 
		void see();

		/// @brief Returns the tuple of selected items.
		std::vector<std::string> selection()
		{
			return this->tk->call<std::vector<std::string>>(this->_w, "selection");
		}

	private:
		/// @brief internal.
		void _selection(const std::string& selop, detail::item_arg auto&&...items)
		{
			this->tk->call(this->_w, "selection", selop, detail::to_item_arg(items)...);
		}
		/// @copydoc _selection
		void _selection(const std::string& selop, detail::range_of_item_arg auto&& items)
		{
			this->tk->call(this->_w, "selection", selop, items | std::views::transform(detail::to_item_arg));
		}

	public:
		/// @brief The specified items becomes the new selection.
		void selection_set(detail::item_arg auto&&...items)
		{
			this->_selection("set", items...);
		}
		void selection_set(detail::range_of_item_arg auto&& items)
		{
			this->_selection("set", items);
		}

		/// @brief Add all of the specified items to the selection.
		void selection_add(detail::item_arg auto&&...items)
		{
			this->_selection("add", items...);
		}
		void selection_add(detail::range_of_item_arg auto&& items)
		{
			this->_selection("add", items);
		}

		/// @brief Remove all of the specified items from the selection.
		void selection_remove(detail::item_arg auto&&...items)
		{
			this->_selection("remove", items...);
		}
		void selection_remove(detail::range_of_item_arg auto&& items)
		{
			this->_selection("remove", items);
		}

		/// @brief Toggle the selection state of each specified item.
		void selection_toggle(detail::item_arg auto&&...items)
		{
			this->_selection("toggle", items...);
		}
		void selection_toggle(detail::range_of_item_arg auto&& items)
		{
			this->_selection("toggle", items);
		}

		/// @brief 
		void set();

		/// @brief 
		void tag_bind();

		/// @brief 
		void tag_configure();

		/// @brief 
		void tag_has();
	};
}