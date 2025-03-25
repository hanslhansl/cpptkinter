module;
#include "../global.hpp"
export module cpptkinter:cpptkinter.widget.base;
import :utility;
import :_cpptkinter;
import :cpptkinter.detail;
import :cpptkinter.misc;
import :cpptkinter.cnfs;
import std;

export namespace cpptkinter::detail
{
    /// @brief Return type of Pack::pack_info().
    ///
    /// @see cnfs::pack_configure
    struct PackInfo
    {
        std::string anchor;
        bool expand;
        std::string fill;
        Misc in;
        long long ipadx;
        long long ipady;
        long long padx;
        long long pady;
        std::string side;
    };

    /// @brief Return type of Place::place_info().
    ///
    /// @see cnfs::place_configure
    struct PlaceInfo
    {
        Misc in;
        std::string x;
        std::string relx;
        std::string y;
        std::string rely;
        std::string width;
        std::string relwidth;
        std::string height;
        std::string relheight;
        std::string anchor;
        std::string bordermode;
    };

    /// @brief Return type of Grid::grid_info().
    ///
    /// @see cnfs::grid_configure
    struct GridInfo
    {
        Misc in;
        long long column;
        long long row;
        long long columnspan;
        long long rowspan;
        long long ipadx;
        long long ipady;
        long long padx;
        long long pady;
        std::string sticky;
    };
}

export namespace cpptkinter
{
    /// @brief Mix-in class for querying and changing the horizontal position of a widget's window.
    /// 
    /// Implemented as crtp to enable the use of utility::member_functor.
    template<typename Self>
    struct XView
    {
        struct : utility::member_functor<Misc::impl>
        {
            using decays_to = void(const std::vector<Tcl_Obj>&);

            std::array<double, 2> operator()()
            {
                return self.tk->template call<std::array<double, 2>>(self._w, "xview");
            }

            void operator()(const std::vector<Tcl_Obj>& args)
            {
                self.tk->call(self._w, "xview", args);
            }
        }
        /// @brief Query or change the horizontal position of the view.
        xview{ *static_cast<Self*>(this)->pimpl };

        /// @brief Adjusts the view in the window so that FRACTION of the total width of the canvas is off - screen to the left.
        void xview_moveto(double fraction) const
        {
            static_cast<const Self*>(this)->tk->call(static_cast<const Self*>(this)->_w, "xview", "moveto", fraction);
        }

        /// @brief Shift the x-view according to NUMBER which is measured in "units" or "pages" (WHAT).
        void xview_scroll(detail::screenunits_arg auto&& number, const std::string& what) const
        {
            static_cast<const Self*>(this)->tk->call(static_cast<const Self*>(this)->_w, "xview", "scroll", detail::to_screenunits_arg(number), what);
        }
    };

    /// @brief Mix-in class for querying and changing the vertical position of a widget's window.
    /// 
    /// Implemented as crtp to enable the use of utility::member_functor.
    template<typename Self>
    struct YView
    {
        struct : utility::member_functor<Misc::impl>
        {
            using decays_to = void(const std::vector<Tcl_Obj>&);

            std::array<double, 2> operator()()
            {
                return self.tk->template call<std::array<double, 2>>(self._w, "yview");
            }

            void operator()(const std::vector<Tcl_Obj>& args)
            {
                self.tk->call(self._w, "yview", args);
            }
        }
        /// @brief Query or change the vertical position of the view.
        yview{ *static_cast<Self*>(this)->pimpl };

        /// @brief Adjusts the view in the window so that FRACTION of the total height of the canvas is off - screen to the top.
        void yview_moveto(double fraction) const
        {
            static_cast<const Self*>(this)->tk->call(static_cast<const Self*>(this)->_w, "yview", "moveto", fraction);
        }

        /// @brief Shift the y-view according to NUMBER which is measured in "units" or "pages" (WHAT).
        void yview_scroll(detail::screenunits_arg auto&& number, const std::string& what) const
        {
            static_cast<const Self*>(this)->tk->call(static_cast<const Self*>(this)->_w, "yview", "scroll", detail::to_screenunits_arg(number), what);
        }
    };
    
    /// @brief Geometry manager Pack.
    /// 
    /// Base class to use the methods pack_* in every widget.
    struct Pack
    {
        /// @brief %Pack a widget in the parent widget.
        template<cnfs::is_cnf CNF = cnfs::pack_configure>
        void pack_configure(this auto&& self, CNF&& cnf = {})
        {
            self.tk->call("pack", "configure", self._w, self._options(std::forward<CNF>(cnf)));
        }
        /// @copydoc pack_configure
        template<cnfs::is_cnf CNF = cnfs::pack_configure>
        void pack(this auto&& self, CNF&& cnf = {})
        {
            return self.pack_configure(std::forward<CNF>(cnf));
        }

        /// @brief Unmap this widget and do not use it for the packing order.
        void pack_forget(this auto&& self)
        {
            self.tk->template call<void>("pack", "forget", self._w);
        }

        /// Return information about the packing options for this widget.
        detail::PackInfo pack_info(this auto&& self)
        {
            return detail::pack_grid_info<detail::PackInfo>(self, "pack", "info", self._w);
        }
    };

    /// @brief Geometry manager Place.
    ///
    /// Base class to use the methods place_* in every widget.
    struct Place
    {
        /// @brief %Place a widget in the parent widget.
        template<cnfs::is_cnf CNF = cnfs::place_configure>
        void place_configure(this auto&& self, CNF&& cnf = {})
        {
            self.tk->call("place", "configure", self._w, self._options(std::forward<CNF>(cnf)));
        }
        /// @copydoc place_configure
        template<cnfs::is_cnf CNF = cnfs::place_configure>
        void place(this auto&& self, CNF&& cnf = {})
        {
            self.place_configure(std::forward<CNF>(cnf));
        }

        // @brief Unmap this widget.
        void place_forget(this auto&& self)
        {
            self.tk->call("place", "forget", self._w);
        }

        detail::PlaceInfo place_info(this auto&& self)
        {
            auto str = self.tk->template call<std::string>("place", "info", self._w);
            std::vector<std::string> vec = self.tk->splitlist(str);
            std::map<std::string, std::string> map(std::from_range, std::views::zip(
                vec | /*std::views::*/stride(2),
                vec | /*std::views::*/drop(1) | /*std::views::*/stride(2)
            ));

            auto converter = [&self]<typename T2>(std::string && v)->T2
            {
                if constexpr (std::same_as<T2, Misc>)
                    return self.Misc::nametowidget(std::move(v));
                else
                    return std::move(v);
            };

            return detail::_splitdict_to_aggregate<detail::PlaceInfo>(std::move(map), true, converter);
        }
    };

    /// @brief Geometry manager Grid.
    /// 
    /// Base class to use the methods grid_* in every widget.
    struct Grid
    {
        /// @brief Position a widget in the parent widget in a grid.
        template<cnfs::is_cnf CNF = cnfs::grid_configure>
        void grid_configure(this auto&& self, CNF&& cnf = {})
        {
            self.tk->call("grid", "configure", self._w, self._options(std::forward<CNF>(cnf)));
        }
        /// @copydoc grid_configure
        template<cnfs::is_cnf CNF = cnfs::grid_configure>
        void grid(this auto&& self, CNF&& cnf = {})
        {
            self.grid_configure(std::forward<CNF>(cnf));
        }

        /// @brief Unmap this widget.
        void grid_forget(this auto&& self)
        {
            self.tk->call("grid", "forget", self._w);
        }

        /// @brief Unmap this widget but remember the grid options.
        void grid_remove(this auto&& self)
        {
            self.tk->call("grid", "remove", self._w);
        }

        /// @brief Return information about the options for positioning this widget in a grid.
        detail::GridInfo grid_info(this auto&& self)
        {
            return detail::pack_grid_info<detail::GridInfo>(self, "grid", "info", self._w);
        }
    };

    /// @brief Internal class.
    class BaseWidget : public Misc
    {
    protected:
        struct impl : Misc::impl
        {
            std::string widgetName;
            std::string _name;

            /// @brief Destroy this widget and all descendants.
            void destroy() override
            {
                // keeps this from being destroyed before this function returns
                auto temp = this->shared_from_this();

                for (auto&& child : std::vector(std::from_range, std::views::values(this->children)))
                    child.destroy();
                this->tk->call("destroy", this->_w);
                this->master.value().children.erase(this->_name);
                this->Misc::impl::destroy();
            }
        };

    public:
        REF_TO_IMPL(widgetName);
    protected:
        REF_TO_IMPL(_name);

        IMPL_CTOR(BaseWidget, Misc);
        DEFINE_ASSIGNMENT_OPERATOR(BaseWidget)

            /// @brief Internal function. Sets up information about children.
            ///
            /// @param override_name only used by Checkbutton.
            template<typename Self>
        void _setup(this Self&& self, const std::optional<Misc>& master_, auto& cnf, std::set<std::string>& ignore_fields, const std::optional<std::string>& override_name = std::nullopt)
        {
            auto&& master = master_.has_value() ? master_.value() : detail::_get_default_root();
            self.master = master;
            self.tk = master.tk;

            std::string name{};
            if (override_name.has_value())
            {
                name = override_name.value();
                ignore_fields.insert("name");
            }
            else if constexpr (requires { cnf.name; })
            {
                utility::invoke_or_and_then([&ignore_fields, &name](auto& v) {
                    name = v;
                    ignore_fields.insert("name");
                    }, cnf.name);
            }
            if (name.empty())
            {
                name = hhh::misc::to_lower(reflect::type_name<Self>());

                auto count = master._last_child_ids[name] += 1;

                if (count == 1)
                    name = std::format("!{}", name);
                else
                    name = std::format("!{}{}", name, count);
            }
            self._name = name;
            if (master._w == ".")
                self._w = "." + name;
            else
                self._w = master._w + "." + name;

            auto&& [it, success] = master.children.emplace(self._name, self);
            if (!success)
            {
                it->second.destroy();
                it->second = self;
            }
        }

        /// @brief Initialize a widget.
        ///
        /// master is passed within cnf instead of as a separate argument.
        /// @param widgetName is the name of the widget.
        /// @param cnf is a cnf structure of options to configure the widget.
        /// @param extra is an optional std::vector of additional options to configure the widget.
        /// @param ignore_fields is an optional std::set of fields to ignore in cnf.
        /// @param pimpl is an optional shared pointer to the implementation. Used by derived classes that extend impl.
        template<cnfs::is_cnf CNF>
        void _init_(this auto&& self, const std::string& widgetName, CNF&& cnf, std::vector<_cpptkinter::Tcl_Obj>&& extra = {}, std::set<std::string> ignore_fields = {})
        {
            std::optional<Misc> master{};
            if constexpr (requires { cnf.master; })
            {
                utility::invoke_or_and_then([&master]<typename T>(T && v) {
                    master = std::forward<T>(v);
                }, std::forward<CNF>(cnf).master);
                ignore_fields.insert("master");
            }

            self.widgetName = widgetName;
            self._setup(master, cnf, ignore_fields);
            self.tk->call(self.widgetName, self._w, std::move(extra), self._options(std::forward<CNF>(cnf), ignore_fields));
            DEVIATING_IMPLEMENTATION_WARNING("something with classes in cnf (?)");
        }
    };

    /// @brief Internal class.
    /// 
    /// Base class for a widget which can be positioned with the geometry managers Pack, Place or Grid.
    struct Widget : BaseWidget, Pack, Grid, Place
    {
    protected:
        IMPL_CTOR(Widget, BaseWidget);
    public:

        /// @brief Exists only to make reflect work.
        Widget() : BaseWidget(std::make_shared<impl>()) { ANNOTATION_WARNING("Exists only to make reflect work."); }
    };
}