module;
#include "../global.hpp"
export module cpptkinter:ttk;
export import :ttk.cnfs;
import :utility;
import :cpptkinter.widget.base;
import :cpptkinter.misc;
import :cpptkinter.cnfs;
import std;
import hhh;

export namespace cpptkinter::detail
{
	/// @brief std::string or std::size_t
	template<typename T>
	concept tab_id_arg = utility::union_arg<T, std::size_t, std::string>;
	constexpr auto to_tab_id = utility::to_union_arg<std::size_t, std::string>;

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
			this->tk->call(this->_w, "forget", detail::to_tab_id(tab_id));
		}

		/// @brief Hides the tab specified by tab_id.
		/// 
		/// The tab will not be displayed, but the associated window remains managed by the notebook and its configuration remembered.
		/// Hidden tabs may be restored with the add command.
		void hide(detail::tab_id_arg auto&& tab_id)
		{
			this->tk->call(this->_w, "hide", detail::to_tab_id(tab_id));
		}

		/// @brief Returns the numeric index of the tab specified by tab_id, or the total number of tabs if tab_id is the string "end".
		std::size_t index(detail::tab_id_arg auto&& tab_id)
		{
			return this->tk->call<std::size_t>(this->_w, "index", detail::to_tab_id(tab_id));
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
			this->tk->call(this->_w, "select", detail::to_tab_id(tab_id));
		}

		/// @brief Query the options of the specific tab_id.
		detail::Notebook_tab_return tab(detail::tab_id_arg auto&& tab_id)
		{
			using V = std::variant<long long, std::string, std::vector<long long>>;
			using M = std::map<std::string, V>;

			auto vec = this->tk->call<std::vector<V>>(this->_w, "tab", detail::to_tab_id(tab_id));
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
}