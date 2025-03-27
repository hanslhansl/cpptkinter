module;
#include "../global.hpp"
export module cpptkinter:ttk;
import :utility;
import :cpptkinter.widget.base;
import std;
import hhh;

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

	};
}