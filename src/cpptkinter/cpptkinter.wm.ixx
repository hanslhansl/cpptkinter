module;
#include "../global.hpp"
#include <range/v3/all.hpp>
export module cpptkinter:cpptkinter.wm;
import :_cpptkinter;
import std;


#if defined(__cpp_lib_ranges_stride) && defined(__cpp_lib_ranges_to_container) && defined(__cpp_lib_ranges) && defined(__cpp_lib_ranges_zip) && defined(__cpp_lib_ranges_join_with)
using std::views::stride;
using std::ranges::to;
using std::views::drop;
using std::views::zip;
using std::ranges::join_with_view;
#else
using ranges::views::stride;
using ranges::to;
using ranges::views::drop;
using ranges::views::zip;
using ranges::join_with_view;
#endif

export namespace cpptkinter
{
    class Misc;

    /// @brief Provides functions for the communication with the window manager.
    struct Wm
    {
        /// @brief Instruct the window manager to set the aspect ratio (width/height) of this widget to be between MINNUMER / MINDENOM and MAXNUMER / MAXDENOM.
        void wm_aspect(this auto&& self, long long minNumer, long long minDenom, long long maxNumer, long long maxDenom)
        {
            self.tk->call("wm", "aspect", self._w, minNumer, minDenom, maxNumer, maxDenom);
        }
        /// @brief Removes any aspect ratio restrictions.
        ///
        /// Should only be called with 4 empty strings.
        void wm_aspect(this auto&& self, const std::string& minNumer, const std::string& minDenom, const std::string& maxNumer, const std::string& maxDenom)
        {
            self.tk->call("wm", "aspect", self._w, minNumer, minDenom, maxNumer, maxDenom);
        }
        /// @brief Returns the current aspect ratio restriction (if any).
        std::optional<std::array<long long, 4>> wm_aspect(this auto&& self)
        {
            auto res = self.tk->template call<std::variant<std::array<long long, 4>, std::string>>("wm", "aspect", self._w);
            if (std::holds_alternative<std::string>(res))
                return {};
            return std::get<std::array<long long, 4>>(res);
        }
        /// @copydoc wm_aspect(this auto&&, long long, long long, long long, long long)
        void aspect(this auto&& self, long long minNumer, long long minDenom, long long maxNumer, long long maxDenom)
        {
            self.wm_aspect(minNumer, minDenom, maxNumer, maxDenom);
        }
        /// @copydoc wm_aspect(this auto&&, const std::string&, const std::string&, const std::string&, const std::string&)
        void aspect(this auto&& self, const std::string& minNumer, const std::string& minDenom, const std::string& maxNumer, const std::string& maxDenom)
        {
            self.wm_aspect(minNumer, minDenom, maxNumer, maxDenom);
        }
        /// @copydoc wm_aspect(this auto&&)
        std::optional<std::array<long long, 4>> aspect(this auto&& self)
        {
            return self.wm_aspect();
        }

        /// @brief This subcommand returns or sets platform specific attributes
        ///
        /// The first form returns a list of the platform specific flags and their values.
        /// The second form returns the value for the specific option.
        /// The third form sets one or more of the values. The values are as follows: 
        /// 
        /// On Windows,
        /// - disabled gets or sets whether the window is in a disabled state.
        /// - toolwindow gets or sets the style of the window to toolwindow (as defined in the MSDN).
        /// - topmost gets or sets whether this is a topmost window (displays above all other windows).
        ///
        /// On Macintosh, XXXXX
        ///
        /// On Unix, there are currently no special attribute values.
        std::map<std::string, std::variant<std::string, double, long long>> wm_attributes(this auto&& self)
        {
            using V = std::variant<std::string, double, long long>;
            auto data = self.tk->template call<std::vector<V>>("wm", "attributes", self._w);

            auto lambda = [](V& var) {
                auto&& key = std::get<std::string>(std::move(var));
                if (key.starts_with('-'))
                    key = key.substr(1);
                return key;
                };

            return std::map<std::string, V>(std::from_range, std::views::zip(
                data | /*std::views::*/stride(2) | std::views::transform(lambda),
                data | std::views::drop(1) | /*std::views::*/stride(2)
            ));
        }
        /// @copydoc wm_attributes(this auto&&)
        std::map<std::string, std::variant<std::string, double, long long>> attributes(this auto&& self)
        {
            return self.wm_attributes();
        }

        /// Store NAME in WM_CLIENT_MACHINE property of this widget. Return current value.
        void wm_client(this auto&& self, const std::string& name)
        {
            self.tk->call("wm", "client", self._w, name);
        }
        /// Get the last name set in a wm client command
        std::string wm_client(this auto&& self)
        {
            return self.tk->template call<std::string>("wm", "client", self._w);
        }
        /// @copydoc wm_client(this auto&&, const std::string&)
        void client(this auto&& self, const std::string& name)
        {
            self.wm_client(name);
        }
        /// @copydoc wm_client(this auto&&)
        std::string client(this auto&& self)
        {
            return self.wm_client();
        }

        /// @brief Store list of window names (WLIST) into WM_COLORMAPWINDOWS property of this widget.
        /// 
        /// This list contains windows whose colormaps differ from their parents. Return current list of widgets if WLIST is empty.
        void wm_colormapwindows(this auto&& self);
        /// @copydoc wm_colormapwindows
        void colormapwindows(this auto&& self);

        /// @brief Store VALUE in WM_COMMAND property.
        /// 
        /// It is the command which shall be used to invoke the application.Return current command if VALUE is None.
        void wm_command();
        /// @copydoc wm_command
        void command();

        /// @brief Deiconify this widget.
        /// 
        /// If it was never mapped it will not be mapped. On Windows it will raise this widget and give it the focus.
        void wm_deiconify(this auto&& self)
        {
            self.tk->call("wm", "deiconify", self._w);
        }
        /// @copydoc wm_deiconify
        void deiconify(this auto&& self)
        {
            self.wm_deiconify();
        }

        /// @brief Set focus model.
        /// 
        /// "active" means that this widget will claim the focus itself, "passive" means that the window manager shall give the focus.
        void wm_focusmodel(this auto&& self, const std::string& model)
        {
            self.tk->call("wm", "focusmodel", self._w, model);
        }
        /// @brief Return current focus model.
        std::string wm_focusmodel(this auto&& self)
        {
            return self.tk->template call<std::string>("wm", "focusmodel", self._w);
        }
        /// @copydoc wm_focusmodel(this auto&&, const std::string&)
        void focusmodel(this auto&& self, const std::string& model)
        {
            return self.wm_focusmodel(model);
        }
        /// @copydoc wm_focusmodel(this auto&&)
        std::string focusmodel(this auto&& self)
        {
            return self.wm_focusmodel();
        }

        /// @brief The window will be unmapped from the screen and will no longer be managed by wm.
        /// 
        /// Toplevel windows will be treated like frame windows once they are no longer managed by wm, however,
        /// the menu option configuration will be remembered and the menus will return once the widget is managed again.
        void wm_forget(this auto&& self, const std::derived_from<Misc> auto& window)
        {
            self.tk->call("wm", "forget", window);
        }
        /// @copydoc wm_forget
        void forget(this auto&& self, const std::derived_from<Misc> auto& window)
        {
            self.wm_forget(window);
        }

        /// Return identifier for decorative frame of this widget if present.
        void wm_frame(this auto&& self)
        {
            self.tk->call("wm", "frame", self._w);
        }
        /// @copydoc wm_frame
        void frame(this auto&& self)
        {
            self.wm_frame();
        }

        /// @brief Set geometry to NEWGEOMETRY of the form =widthxheight+x+y.
        void wm_geometry(this auto&& self, const std::string& newGeometry)
        {
            self.tk->call("wm", "geometry", self._w, newGeometry);
        }
        /// @brief Get current geometry.
        std::string wm_geometry(this auto&& self)
        {
            return self.tk->template call<std::string>("wm", "geometry", self._w);
        }
        /// @copydoc wm_geometry(this auto&&, const std::string&)
        void geometry(this auto&& self, const std::string& newGeometry)
        {
            return self.wm_geometry(newGeometry);
        }
        /// @copydoc wm_geometry(this auto&&)
        std::string geometry(this auto&& self)
        {
            return self.wm_geometry();
        }

        /// @brief Manage window as a gridded window.
        /// 
        /// WIDTHINC and HEIGHTINC are the width and height of a grid unit in pixels. BASEWIDTH and BASEHEIGHT are the number of grid units requested in Tk_GeometryRequest.
        void wm_grid(this auto&& self, long long baseWidth, long long baseHeight, long long widthInc, long long heightInc)
        {
            self.tk->call("wm", "grid", self._w, baseWidth, baseHeight, widthInc, heightInc);
        }
        /// @brief Window will no longer be managed as a gridded window
        ///
        /// Should only be called with 4 empty strings.
        void wm_grid(this auto&& self, const std::string& baseWidth, const std::string& baseHeight, const std::string& widthInc, const std::string& heightInc)
        {
            self.tk->call("wm", "grid", self._w, baseWidth, baseHeight, widthInc, heightInc);
        }
        /// @brief Return grid information for this widget.
        std::optional<std::array<long long, 4>> wm_grid(this auto&& self)
        {
            auto res = self.tk->template call<std::variant<std::array<long long, 4>, std::string>>("wm", "grid", self._w);
            if (std::holds_alternative<std::string>(res))
                return {};
            return std::get<std::array<long long, 4>>(res);
        }
        /// @copydoc wm_grid(this auto&&, long long, long long, long long, long long)
        void grid(this auto&& self, long long baseWidth, long long baseHeight, long long widthInc, long long heightInc)
        {
            self.wm_grid(baseWidth, baseHeight, widthInc, heightInc);
        }
        /// @copydoc wm_grid(this auto&&, const std::string&, const std::string&, const std::string&, const std::string&)
        void grid(this auto&& self, const std::string& baseWidth, const std::string& baseHeight, const std::string& widthInc, const std::string& heightInc)
        {
            self.wm_grid(baseWidth, baseHeight, widthInc, heightInc);
        }
        /// @copydoc wm_grid(this auto&&)
        std::optional<std::array<long long, 4>> grid(this auto&& self)
        {
            return self.wm_grid();
        }

        /// @brief Set the group leader widgets for related widgets to PATHNAME.
        void wm_group(this auto&& self, const std::string& pathName)
        {
            self.tk->call("wm", "group", self._w, pathName);
        }
        /// @brief Get the current group leader.
        std::string wm_group(this auto&& self)
        {
            return self.tk->template call<std::string>("wm", "group", self._w);
        }
        /// @copydoc wm_group(this auto&&, const std::string&)
        void group(this auto&& self, const std::string& pathName)
        {
            self.wm_group(pathName);
        }
        /// @copydoc wm_group(this auto&&)
        std::string group(this auto&& self)
        {
            return self.wm_group();
        }

        /// @brief Set bitmap for the iconified widget to BITMAP.
        ///
        /// Under Windows, the DEFAULT parameter can be used to set the icon for the widget and any descendants that don't have an icon set explicitly.
        /// See Tk documentation for more information.
        void wm_iconbitmap(this auto&& self, const std::string& bitmap, bool default_)
        {
            if (default_)
                self.tk->call("wm", "iconmask", self._w, "-default", bitmap);
            else
                self.tk->call("wm", "iconmask", self._w, bitmap);
        }
        /// @brief Get name of the current icon bitmap associated with window
        std::string wm_iconbitmap(this auto&& self)
        {
            return self.tk->template call<std::string>("wm", "iconmask", self._w);
        }
        /// @copydoc wm_iconbitmap(this auto&&, const std::string&, bool)
        void iconbitmap(this auto&& self, const std::string& bitmap, bool default_)
        {
            self.wm_iconbitmap(bitmap, default_);
        }
        /// @copydoc wm_iconbitmap(this auto&&)
        std::string iconbitmap(this auto&& self)
        {
            return self.wm_iconbitmap();
        }

        /// @brief Display widget as icon.
        void wm_iconify(this auto&& self)
        {
            self.tk->call("wm", "iconify", self._w);
        }
        /// @copydoc wm_iconify
        void iconify(this auto&& self)
        {
            self.wm_iconify();
        }

        /// Set mask for the icon bitmap of this widget.
        void wm_iconmask(this auto&& self, const std::string& bitmap)
        {
            self.tk->call("wm", "iconmask", self._w, bitmap);
        }
        /// Get the current mask for the icon bitmap.
        std::string wm_iconmask(this auto&& self)
        {
            return self.tk->template call<std::string>("wm", "iconmask", self._w);
        }
        /// @copydoc wm_iconmask(this auto&&, const std::string&)
        void iconmask(this auto&& self, const std::string& bitmap)
        {
            return self.wm_iconmask(bitmap);
        }
        /// @copydoc wm_iconmask(this auto&&)
        std::string iconmask(this auto&& self)
        {
            return self.wm_iconmask();
        }

        /// @brief Set the name of the icon for this widget.
        void wm_iconname(this auto&& self, const std::string& newName)
        {
            self.tk->call("wm", "iconname", self._w, newName);
        }
        /// @brief Return the name of the icon for this widget.
        std::string wm_iconname(this auto&& self)
        {
            return self.tk->template call<std::string>("wm", "iconname", self._w);
        }
        /// @copydoc wm_iconname(this auto&&, const std::string&)
        void iconname(this auto&& self, const std::string& newName)
        {
            return self.wm_iconname(newName);
        }
        /// @copydoc wm_iconname(this auto&&)
        std::string iconname(this auto&& self)
        {
            return self.wm_iconname();
        }

        /// @brief
        void wm_iconphoto();
        void iconphoto();

        /// @brief Set the position of the icon of this widget to X and Y. 
        void wm_iconposition(this auto&& self, long long x, long long y)
        {
            self.tk->call("wm", "iconposition", self._w, x, y);
        }
        /// @brief Return the current position of the icon of this widget.
        std::array<long long, 2> wm_iconposition(this auto&& self)
        {
            return self.tk->template call<std::array<long long, 2>>("wm", "iconposition", self._w);
        }
        /// @copydoc wm_iconposition(this auto&&, long long, long long)
        void iconposition(this auto&& self, long long x, long long y)
        {
            return self.wm_iconposition(x, y);
        }
        /// @copydoc wm_iconposition(this auto&&)
        std::array<long long, 2> iconposition(this auto&& self)
        {
            return self.wm_iconposition();
        }

        /// @brief Set widget PATHNAME to be displayed instead of icon.
        void wm_iconwindow(this auto&& self, const std::string& pathName)
        {
            self.tk->call("wm", "wm_iconwindow", self._w, pathName);
        }
        /// @brief Return the current value of the icon window.
        Misc wm_iconwindow(this auto&& self);
        /// @copydoc wm_iconwindow(this auto&&, const std::string&)
        void iconwindow(this auto&& self, const std::string& pathName)
        {
            return self.wm_iconwindow(pathName);
        }
        /// @copydoc wm_iconwindow(this auto&&)
        Misc iconwindow(this auto&& self);

        /// @brief The widget specified will become a stand alone top-level window. 
        /// 
        /// The window will be decorated with the window managers title bar, etc.
        void wm_manage(this auto&& self, const std::derived_from<Misc> auto& widget)
        {
            self.tk->call("wm", "manage", widget);
        }
        /// @copydoc wm_manage
        void manage(this auto&& self, const std::derived_from<Misc> auto& widget)
        {
            return self.wm_manage(widget);
        }

        /// @brief Set max WIDTH and HEIGHT for this widget. If the window is gridded the values are given in grid units.
        void wm_maxsize(this auto&& self, long long width, long long height)
        {
            self.tk->call("wm", "maxsize", self._w, width, height);
        }
        /// @brief Return the current max WIDTH and HEIGHT for this widget.
        std::array<long long, 2> wm_maxsize(this auto&& self)
        {
            return self.tk->template call<std::array<long long, 2>>("wm", "maxsize", self._w);
        }
        /// @copydoc wm_maxsize(this auto&&, long long, long long)
        void maxsize(this auto&& self, long long width, long long height)
        {
            return self.wm_maxsize(width, height);
        }
        /// @copydoc wm_maxsize(this auto&&)
        std::array<long long, 2> maxsize(this auto&& self)
        {
            return self.wm_maxsize();
        }

        /// @brief Set min WIDTH and HEIGHT for this widget. If the window is gridded the values are given in grid units.
        void wm_minsize(this auto&& self, long long width, long long height)
        {
            self.tk->call("wm", "minsize", self._w, width, height);
        }
        /// @brief Return the current min WIDTH and HEIGHT for this widget.
        std::array<long long, 2> wm_minsize(this auto&& self)
        {
            return self.tk->template call<std::array<long long, 2>>("wm", "minsize", self._w);
        }
        /// @copydoc wm_minsize(this auto&&, long long, long long)
        void minsize(this auto&& self, long long width, long long height)
        {
            return self.wm_minsize(width, height);
        }
        /// @copydoc wm_minsize(this auto&&)
        std::array<long long, 2> minsize(this auto&& self)
        {
            return self.wm_minsize();
        }

        /// @brief Instruct the window manager to ignore this widget if BOOLEAN is true.
        void wm_overrideredirect(this auto&& self, bool boolean)
        {
            self.tk->call("wm", "overrideredirect", self._w, boolean);
        }
        /// @brief Return the current value of the overrideredirect flag.
        bool wm_overrideredirect(this auto&& self)
        {
            return self.tk->template call<bool>("wm", "overrideredirect", self._w);
        }
        /// @copydoc wm_overrideredirect(this auto&&, bool)
        void overrideredirect(this auto&& self, bool boolean)
        {
            return self.wm_overrideredirect(boolean);
        }
        /// @copydoc wm_overrideredirect(this auto&&)
        bool overrideredirect(this auto&& self)
        {
            return self.wm_overrideredirect();
        }

        /// @brief Instruct the window manager that the position of this widget shall be defined by the user if WHO is "user", and by its own policy if WHO is "program".
        void wm_positionfrom(this auto&& self, const std::string& who)
        {
            self.tk->call("wm", "positionfrom", self._w, who);
        }
        /// @brief Return the current positionfrom setting.
        std::string wm_positionfrom(this auto&& self)
        {
            return self.tk->template call<std::string>("wm", "positionfrom", self._w);
        }
        /// @copydoc wm_positionfrom(this auto&&, const std::string&)
        void positionfrom(this auto&& self, const std::string& who)
        {
            return self.wm_positionfrom(who);
        }
        /// @copydoc wm_positionfrom(this auto&&)
        std::string positionfrom(this auto&& self)
        {
            return self.wm_positionfrom();
        }

        /// Bind function FUNC to command NAME for this widget.
        /// 
        /// Return the function bound to NAME if None is given. NAME could be e.g. "WM_SAVE_YOURSELF" or "WM_DELETE_WINDOW".
        template<detail::FromObjConcept R = void, typename Func>
            requires detail::createcommand_concept<Func> || detail::AsObjConcept<std::remove_cvref_t<Func>>
        R wm_protocol(this auto && self, const std::string & name, Func && func)
        {
            if constexpr (detail::createcommand_concept<Func>)
                return self.tk->template call<R>("wm", "protocol", self._w, name, self._register(std::forward<Func>(func)));
            else
                return self.tk->template call<R>("wm", "protocol", self._w, name, std::forward<Func>(func));
        }
        /// @copydoc wm_protocol
        template<detail::FromObjConcept R = void, typename Func>
            requires detail::createcommand_concept<Func> || detail::AsObjConcept<std::remove_cvref_t<Func>>
        R protocol(this auto && self, const std::string & name, Func && func)
        {
            return self.template wm_protocol<R>(name, std::forward<Func>(func));
        }

        /// @brief Instruct the window manager whether this width can be resized in WIDTH or HEIGHT.
        void wm_resizable(this auto&& self, bool width, bool height)
        {
            self.tk->call("wm", "resizable", self._w, width, height);
        }
        /// @brief Return the current resizable settings.
        std::array<bool, 2> wm_resizable(this auto&& self)
        {
            return self.tk->template call<std::array<bool, 2>>("wm", "resizable", self._w);
        }
        /// @copydoc wm_resizable(this auto&&, bool, bool)
        void resizable(this auto&& self, bool width, bool height)
        {
            return self.wm_resizable(width, height);
        }
        /// @copydoc wm_resizable(this auto&&)
        std::array<bool, 2> resizable(this auto&& self)
        {
            return self.wm_resizable();
        }

        /// @brief Instruct the window manager that the size of this widget shall be defined by the user if WHO is "user", and by its own policy if WHO is "program".
        void wm_sizefrom(this auto&& self, const std::string& who)
        {
            self.tk->call("wm", "sizefrom", self._w, who);
        }
        /// @brief Return the current sizefrom setting.
        std::string wm_sizefrom(this auto&& self)
        {
            return self.tk->template call<std::string>("wm", "sizefrom", self._w);
        }
        /// @copydoc wm_sizefrom(this auto&&, const std::string&)
        void sizefrom(this auto&& self, const std::string& who)
        {
            return self.wm_sizefrom(who);
        }
        /// @copydoc wm_sizefrom(this auto&&)
        std::string sizefrom(this auto&& self)
        {
            return self.wm_sizefrom();
        }

        /// @brief Set the state of this widget as one of normal, icon, iconic (see wm_iconwindow), withdrawn, or zoomed (Windows only).
        void wm_state(this auto&& self, const std::string& newstate)
        {
            self.tk->call("wm", "state", self._w, newstate);
        }
        /// @brief Return the current state of this widget.
        std::string wm_state(this auto&& self)
        {
            return self.tk->template call<std::string>("wm", "state", self._w);
        }
        /// @copydoc wm_state(this auto&&, const std::string&)
        void state(this auto&& self, const std::string& newstate)
        {
            return self.wm_state(newstate);
        }
        /// @copydoc wm_state(this auto&&)
        std::string state(this auto&& self)
        {
            return self.wm_state();
        }

        /// @brief Set the title of this widget.
        void wm_title(this auto&& self, const std::string& string)
        {
            return self.tk->call("wm", "title", self._w, string);
        }
        /// @brief Get the title of this widget.
        std::string wm_title(this auto&& self)
        {
            return self.tk->template call<std::string>("wm", "title", self._w);
        }
        /// @copydoc wm_title(this auto&&, const std::string&)
        void title(this auto&& self, const std::string& string)
        {
            return self.wm_title(string);
        }
        /// @copydoc wm_title(this auto&&)
        std::string title(this auto&& self)
        {
            return self.wm_title();
        }

        /// @brief Instruct the window manager that this widget is transient with regard to widget MASTER.
        void wm_transient(this auto&& self, const std::string& master)
        {
            self.tk->call("wm", "transient", self._w, master);
        }
        /// @brief Return the current transient master.
        Misc wm_transient(this auto&& self);
        /// @copydoc wm_transient(this auto&&, const std::string&)
        void transient(this auto&& self, const std::string& master)
        {
            return self.wm_transient(master);
        }
        /// @copydoc wm_transient(this auto&&)
        Misc transient(this auto&& self);

        /// @brief Withdraw this widget from the screen such that it is unmapped and forgotten by the window manager.
        /// 
        /// Re - draw it with wm_deiconify.
        void wm_withdraw(this auto&& self)
        {
            self.tk->call("wm", "withdraw", self._w);
        }
        /// @copydoc wm_withdraw
        void withdraw(this auto&& self)
        {
            return self.wm_withdraw();
        }
    };
}