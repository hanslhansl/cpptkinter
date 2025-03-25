module;
#include "../global.hpp"
export module cpptkinter:cpptkinter.widget1;
import :utility;
import :_cpptkinter;
import :cpptkinter.detail;
import :cpptkinter.misc;
import :cpptkinter.cnfs;
import :cpptkinter.widget.base;
import std;


export namespace cpptkinter
{
    /// @brief %Menu widget which allows displaying menu bars, pull-down menus and pop-up menus.
    struct Menu : Widget
    {
        friend class OptionMenu;

        /// @brief Construct a menu widget.
        CONSTRUCTORS_AND_ASSIGNMENT(Menu, cnfs::Menu, "menu", Widget);

        /// @brief Post the menu at position X,Y.
        void tk_popup(long long x, long long y)
        {
            this->tk->call("tk_popup", this->_w, x, y);
        }
        /// @brief Post the menu at position X,Y with entry ENTRY.
        void tk_popup(long long x, long long y, long long entry)
        {
            this->tk->call("tk_popup", this->_w, x, y, entry);
        }

        /// @brief Activate entry at INDEX.
        void activate(detail::index auto&& index)
        {
            this->tk->call(this->_w, "activate", detail::to_index(index));
        }

        /// @brief Internal function.
        template<cnfs::is_cnf CNF>
        void add(const std::string& itemType, CNF&& cnf)
        {
            this->tk->call(this->_w, "add", itemType, this->_options(std::forward<CNF>(cnf)));
        }

        /// @brief Add hierarchical menu item.
        template<cnfs::is_cnf CNF = cnfs::Menu_add_cascade>
        void add_cascade(CNF&& cnf = {})
        {
            this->add("cascade", std::forward<CNF>(cnf));
        }

        /// @brief Add checkbutton menu item.
        template<cnfs::is_cnf CNF = cnfs::Menu_add_checkbutton<bool>>
        void add_checkbutton(CNF&& cnf = {})
        {
            this->add("checkbutton", std::forward<CNF>(cnf));
        }

        /// @brief Add command menu item.
        template<cnfs::is_cnf CNF = cnfs::Menu_add_command>
        void add_command(CNF&& cnf = {})
        {
            this->add("command", std::forward<CNF>(cnf));
        }

        /// @brief Add radio menu item.
        template<cnfs::is_cnf CNF = cnfs::Menu_add_radiobutton<int>>
        void add_radiobutton(CNF&& cnf = {})
        {
            this->add("radiobutton", std::forward<CNF>(cnf));
        }

        /// @brief Add separator menu item.
        template<cnfs::is_cnf CNF = cnfs::Menu_add_separator>
        void add_separator(CNF&& cnf = {})
        {
            this->add("separator", std::forward<CNF>(cnf));
        }

        /// @brief Internal function.
        template<cnfs::is_cnf CNF>
        void insert(detail::index auto&& index, const std::string& itemType, CNF&& cnf)
        {
            this->tk->call(this->_w, "insert", detail::to_index(index), itemType, this->_options(std::forward<CNF>(cnf)));
        }

        /// @brief Add hierarchical menu item at INDEX.
        template<cnfs::is_cnf CNF = cnfs::Menu_add_cascade>
        void insert_cascade(detail::index auto&& index, CNF&& cnf = {})
        {
            this->insert(index, "cascade", std::forward<CNF>(cnf));
        }

        /// @brief Add checkbutton menu item at INDEX.
        template<cnfs::is_cnf CNF = cnfs::Menu_add_checkbutton<bool>>
        void insert_checkbutton(detail::index auto&& index, CNF&& cnf = {})
        {
            this->insert(index, "checkbutton", std::forward<CNF>(cnf));
        }

        /// @brief Add command menu item at INDEX.
        template<cnfs::is_cnf CNF = cnfs::Menu_add_command>
        void insert_command(detail::index auto&& index, CNF&& cnf = {})
        {
            this->insert(index, "command", std::forward<CNF>(cnf));
        }

        /// @brief Add radio menu item at INDEX.
        template<cnfs::is_cnf CNF = cnfs::Menu_add_radiobutton<int>>
        void insert_radiobutton(detail::index auto&& index, CNF&& cnf = {})
        {
            this->insert(index, "radiobutton", std::forward<CNF>(cnf));
        }

        /// @brief Add separator at INDEX.
        template<cnfs::is_cnf CNF = cnfs::Menu_add_separator>
        void insert_separator(detail::index auto&& index, CNF&& cnf = {})
        {
            this->insert(index, "separator", std::forward<CNF>(cnf));
        }

        /// @brief Delete menu items at INDEX.
        void delete_(detail::index auto&& index)
        {
            this->delete_(detail::to_index(index), detail::to_index(index));
        }
        /// @brief Delete menu items between INDEX1 and INDEX2 (included).
        void delete_(detail::index auto&& index1_, detail::index auto&& index2_)
        {
            auto&& index1 = detail::to_index(index1_);
            auto&& index2 = detail::to_index(index2_);

            auto num_index1 = this->index(index1);
            auto num_index2 = this->index(index2);

            //if (num_index1 is None) or (num_index2 is None) :
            //    num_index1, num_index2 = 0, -1

            for (long long i = num_index1; i < num_index2 + 1; i++)
            {
                auto ec = this->entryconfig(i);
                if (ec.contains("command"))
                {
                    auto c = this->entrycget<std::string>(i, "command");
                    if (!c.empty())
                        this->deletecommand(c);
                }
            }
            this->tk->call(this->_w, "delete", index1, index2);
        }

        /// @brief Return the resource value of a menu item for OPTION at INDEX.
        template<detail::FromObjConcept R>
        R entrycget(detail::index auto&& index, const std::string& option)
        {
            return this->tk->call<R>(this->_w, "entrycget", detail::to_index(index), "-" + option);
        }

        /// @brief Configure a menu item at INDEX.
        template<cnfs::is_cnf CNF>
        auto entryconfigure(detail::index auto&& index_, CNF&& cnf)
        {
            auto&& index = detail::to_index(index_);
            if constexpr (std::same_as<std::remove_cvref_t<decltype(index)>, long long>)
                return this->_configure({ "entryconfigure", std::to_string(index) }, std::forward<CNF>(cnf));
            else
                return this->_configure({ "entryconfigure", index }, std::forward<CNF>(cnf));
        }
        /// @brief Configure a menu item at INDEX.
        auto entryconfigure(detail::index auto&& index_) -> decltype(this->_configure({}))
        {
            auto&& index = detail::to_index(index_);
            if constexpr (std::same_as<std::remove_cvref_t<decltype(index)>, long long>)
                return this->_configure({ "entryconfigure", std::to_string(index) });
            else
                return this->_configure({ "entryconfigure", index });
        }

        /// @copydoc entryconfigure(long long, CNF&&)
        template<cnfs::is_cnf CNF>
        auto entryconfig(detail::index auto&& index, CNF&& cnf)
        {
            return this->entryconfigure(detail::to_index(index), std::forward<CNF>(cnf));
        }
        /// @copydoc entryconfigure(long long)
        auto entryconfig(detail::index auto&& index) -> decltype(this->entryconfigure(index))
        {
            return this->entryconfigure(detail::to_index(index));
        }

        /// @brief Return the index of a menu item identified by INDEX.
        long long index(detail::index auto&& index)
        {
            return this->tk->call<long long>(this->_w, "index", detail::to_index(index));
        }

        /// @brief Invoke a menu item identified by INDEX and execute the associated command.
        template<detail::FromObjConcept R = void>
        R invoke(detail::index auto&& index)
        {
            return this->tk->call<R>(this->_w, "invoke", detail::to_index(index));
        }

        /// @brief Display a menu at position X,Y.
        void post(long long x, long long y)
        {
            this->tk->call(this->_w, "post", x, y);
        }

        /// @brief Return the type of the menu item at INDEX.
        std::string type(detail::index auto&& index)
        {
            return this->tk->call<std::string>(this->_w, "type", detail::to_index(index));
        }

        /// @brief Unmap a menu.
        void unpost()
        {
            this->tk->call(this->_w, "unpost");
        }

        /// @brief Return the x-position of the leftmost pixel of the menu item at INDEX.
        long long xposition(detail::index auto&& index)
        {
            return this->tk->call<long long>(this->_w, "xposition", detail::to_index(index));
        }

        /// @brief "Return the y-position of the topmost pixel of the menu item at INDEX.
        long long yposition(detail::index auto&& index)
        {
            return this->tk->call<long long>(this->_w, "yposition", detail::to_index(index));
        }
    };

    namespace cnfs
    {
        using opt_menu = opt<cpptkinter::Menu>;

        /// @brief Argument for Menu::add_command().
        struct Menu_add_cascade
        {
            opt_string accelerator;
            opt_string activebackground;
            opt_string activeforeground;
            opt_string background;
            opt_string bitmap;
            opt<long long> columnbreak;
            opt<std::variant<std::string, std::function<void()>>> command;
            opt_compound compound;
            opt_font_description font;
            opt_string foreground;
            opt_bool hidemargin;
            opt_image_spec image;
            opt_string label;
            opt_menu menu;
            opt_string state;
            opt<long long> underline;
        };

        /// @brief Argument for Toplevel::Toplevel().
        struct Toplevel
        {
            opt_master master;
            opt_string background;
            opt_screenunits bd;
            opt_string bg;
            opt_screenunits border;
            opt_screenunits borderwidth;
            opt_string class_;
            opt<std::variant<std::string, Misc>> colormap;
            opt_bool container;
            opt_cursor cursor;
            opt_screenunits height;
            opt_string highlightbackground;
            opt_string highlightcolor;
            opt_screenunits highlightthickness;
            opt_menu menu;
            opt_string name;
            opt_screenunits padx;
            opt_screenunits pady;
            opt_relief relief;
            opt_string screen;
            opt_take_focus_value takefocus;
            opt<std::size_t> use;
            opt_visual_type visual;
            opt_screenunits width;
        };
    }

    /// @brief %Toplevel widget, e.g. for dialogs.
    struct Toplevel : BaseWidget, Wm
    {
    protected:
        template<cnfs::is_cnf CNF>
        void _init_(const std::string& widgetName, CNF&& cnf)
        {
            std::vector<_cpptkinter::Tcl_Obj> extra{};
            std::set<std::string> ignore_fields{};
            reflect::for_each<CNF>([&extra, &ignore_fields, &cnf](auto I) {
                constexpr std::string_view wmkey = reflect::member_name<I, CNF>();

                if constexpr (wmkey == "screen"
                    || wmkey == "class_"
                    || wmkey == "class"
                    || wmkey == "visual"
                    || wmkey == "colormap")
                {
                    std::string opt = "-";
                    if (wmkey.ends_with('_'))
                        opt += wmkey.substr(0, wmkey.size() - 1);
                    else
                        opt += wmkey;


                    utility::invoke_or_and_then([&extra, &opt]<typename T>(T && v) {
                        extra.emplace_back(_cpptkinter::AsObj(opt));
                        extra.emplace_back(_cpptkinter::AsObj(std::forward<T>(v)));
                    }, reflect::get<I>(std::forward<CNF>(cnf)));

                    ignore_fields.insert(std::string(wmkey));
                }
                });

            this->BaseWidget::_init_(widgetName, std::forward<CNF>(cnf), std::move(extra), std::move(ignore_fields));

            auto root = this->_root();
            this->iconname(root.iconname());
            this->title(root.title());
            this->protocol("WM_DELETE_WINDOW", std::function<void()>(std::bind_front(&Toplevel::destroy, *this)));
        }
    public:
        /// @brief Create a new Toplevel widget.
        CONSTRUCTORS_AND_ASSIGNMENT(Toplevel, cnfs::Toplevel, "toplevel", BaseWidget);
    };

    /// @brief %Button widget.
    /// 
    /// @see TypedButton, cpptkinter::Button()
    struct Button : Widget
    {
        /// @brief Construct a new Button widget.
        CONSTRUCTORS_AND_ASSIGNMENT(Button, cnfs::Button, "button", Widget);

        /// @brief Flash the button.
        /// 
        /// This is accomplished by redisplaying the button several times, alternating between active and normal colors.
        /// At the end of the flash the button is left in the same normal/active state as when the command was invoked.
        /// This command is ignored if the button's state is disabled.
        void flash()
        {
            this->tk->call(this->_w, "flash");
        }

        /// @brief Invoke the command associated with the button.
        void invoke()
        {
            this->tk->call(this->_w, "invoke");
        }
    };

    /// @brief %Canvas widget to display graphical elements like lines or text.
    struct Canvas : Widget, XView<Canvas>, YView<Canvas>
    {
        /// @brief Construct a canvas widget.
        CONSTRUCTORS_AND_ASSIGNMENT(Canvas, cnfs::Canvas, "canvas", Widget);

    private:
        /// @brief Internal function.
        template<typename...Args>
        void addtag(Args&&...args)
        {
            this->tk->call(this->_w, "addtag", std::forward<Args>(args)...);
        }
    public:
        /// @brief Add tag NEWTAG to all items above TAGORID.
        void addtag_above(const std::string& newtag, detail::tag_or_id_arg auto&& tagOrId)
        {
            this->addtag(newtag, "above", detail::to_tag_or_id(tagOrId));
        }

        /// @brief Add tag NEWTAG to all items.
        void addtag_all(const std::string& newtag)
        {
            this->addtag(newtag, "all");
        }

        /// @brief Add tag NEWTAG to all items below TAGORID.
        void addtag_below(const std::string& newtag, detail::tag_or_id_arg auto&& tagOrId)
        {
            this->addtag(newtag, "below", detail::to_tag_or_id(tagOrId));
        }

        /// @brief Add tag NEWTAG to item which is closest to pixel at X, Y.
        /// 
        /// If several match take the top - most. All items closer than HALO are considered overlapping (all are closest). If START is specified the next below this tag is taken.
        void addtag_closest(const std::string& newtag, detail::screenunits_arg auto&& x, detail::screenunits_arg auto&& y)
        {
            this->addtag(newtag, "closest", detail::to_screenunits_arg(x), detail::to_screenunits_arg(y));
        }
        /// @copydoc addtag_closest
        void addtag_closest(const std::string& newtag, detail::screenunits_arg auto&& x, detail::screenunits_arg auto&& y, detail::screenunits_arg auto&& halo)
        {
            this->addtag(newtag, "closest", detail::to_screenunits_arg(x), detail::to_screenunits_arg(y), detail::to_screenunits_arg(halo));
        }
        /// @copydoc addtag_closest
        void addtag_closest(const std::string& newtag, detail::screenunits_arg auto&& x, detail::screenunits_arg auto&& y, detail::screenunits_arg auto&& halo,
            utility::union_arg<long long, std::string> auto&& start)
        {
            this->addtag(newtag, "closest", detail::to_screenunits_arg(x), detail::to_screenunits_arg(y), detail::to_screenunits_arg(halo),
                utility::to_union_arg<long long, std::string>(start));
        }

        /// @brief Add tag NEWTAG to all items in the rectangle defined by X1, Y1, X2, Y2.
        void addtag_enclosed(const std::string& newtag, detail::screenunits_arg auto&& x1, detail::screenunits_arg auto&& y1,
            detail::screenunits_arg auto&& x2, detail::screenunits_arg auto&& y2)
        {
            this->addtag(newtag, "enclosed", detail::to_screenunits_arg(x1), detail::to_screenunits_arg(y1), detail::to_screenunits_arg(x2), detail::to_screenunits_arg(y2));
        }

        /// @brief Add tag NEWTAG to all items which overlap the rectangle defined by X1, Y1, X2, Y2.
        void addtag_overlapping(const std::string& newtag, detail::screenunits_arg auto&& x1, detail::screenunits_arg auto&& y1,
            detail::screenunits_arg auto&& x2, detail::screenunits_arg auto&& y2)
        {
            this->addtag(newtag, "overlapping", detail::to_screenunits_arg(x1), detail::to_screenunits_arg(y1), detail::to_screenunits_arg(x2), detail::to_screenunits_arg(y2));
        }

        /// @brief Add tag NEWTAG to all items with TAGORID.
        void addtag_withtag(const std::string& newtag, detail::tag_or_id_arg auto&& tagOrId)
        {
            this->addtag(newtag, "withtag", detail::to_tag_or_id(tagOrId));
        }

        /// @brief Return a tuple of X1,Y1,X2,Y2 coordinates for a rectangle which encloses all items with tags specified as arguments.
        std::array<long long, 4> bbox(detail::tag_or_id_arg auto&&...args) const
        {
            return this->tk->call<std::array<long long, 4>>(this->_w, "bbox", detail::to_tag_or_id(args)...);
        }

        /// @brief Unbind for all items with TAGORID for event SEQUENCE the function identified with FUNCID.
        void tag_unbind(detail::tag_or_id_arg auto&& tagOrId, const std::string& sequence)
        {
            this->_unbind({ this->_w, "bind", std::format("{}", detail::to_tag_or_id(tagOrId)), sequence });
        }
        /// @copydoc tag_unbind
        void tag_unbind(detail::tag_or_id_arg auto&& tagOrId, const std::string& sequence, const std::string& funcid)
        {
            this->_unbind({ this->_w, "bind", std::format("{}", detail::to_tag_or_id(tagOrId)), sequence }, funcid);
        }

        /// @brief Bind to all items with TAGORID at event SEQUENCE a call to function FUNC.
        ///
        /// An additional boolean parameter ADD specifies whether FUNC will be called additionally to the other bound function or whether it will replace the previous function.
        /// See bind for the return value.
        template<typename...Args>
        auto tag_bind(detail::tag_or_id_arg auto&& tagOrId, Args&&...args) requires requires { this->_bind({}, std::forward<Args>(args)...); }
        {
            this->_bind({ this->_w, "bind", std::format("{}", detail::to_tag_or_id(tagOrId)) }, std::forward<Args>(args)...);
        }

        /// @brief Return the canvas x coordinate of pixel position SCREENX rounded to nearest multiple of GRIDSPACING units.
        void canvasx();

        /// @brief Return the canvas y coordinate of pixel position SCREENY rounded to nearest multiple of GRIDSPACING units.
        void canvasy();

        /// @brief Return a list of coordinates for the item given in ARGS.
        void coords();

        /// @brief Internal function.
        template<typename CNF>
        long long _create(const std::string& itemType, const std::vector<double>& args, CNF&& cnf)
        {
            return this->tk->call<long long>(this->_w, "create", itemType, args, this->_options(std::forward<CNF>(cnf)));
        }

        /// @brief Create arc shaped region with coordinates x1,y1,x2,y2.
        template<typename CNF = cnfs::Canvas_create_arc>
        long long create_arc(double x1, double y1, double x2, double y2, CNF&& cnf = {})
        {
            return this->_create("arc", { x1, y1, x2, y2 }, std::forward<CNF>(cnf));
        }

        /// @brief Create bitmap with coordinates x1,y1.
        template<typename CNF = cnfs::Canvas_create_bitmap>
        long long create_bitmap(double x1, double y1, CNF&& cnf = {})
        {
            return this->create_bitmap("bitmap", { x1, y1 }, std::forward<CNF>(cnf));
        }

        /// @brief Create image item with coordinates x1,y1.
        template<typename CNF = cnfs::Canvas_create_image>
        long long create_image(double x1, double y1, CNF&& cnf = {})
        {
            return this->_create("image", { x1, y1 }, std::forward<CNF>(cnf));
        }

        /// @brief Create line with coordinates x1,y1,...,xn,yn.
        template<typename CNF = cnfs::Canvas_create_line>
        long long create_line(const std::vector<std::array<double, 2>>& coords, CNF&& cnf = {})
        {
            return this->_create("line", coords, std::forward<CNF>(cnf));
        }

        /// @brief Create oval with coordinates x1,y1,x2,y2.
        template<typename CNF = cnfs::Canvas_create_oval>
        long long create_oval(double x1, double y1, double x2, double y2, CNF&& cnf = {})
        {
            return this->_create("oval", { x1, y1, x2, y2 }, std::forward<CNF>(cnf));
        }

        /// @brief Create polygon with coordinates x1,y1,...,xn,yn.
        template<typename CNF = cnfs::Canvas_create_polygon>
        long long create_polygon(const std::vector<std::array<double, 2>>& coords, CNF&& cnf = {})
        {
            return this->_create("polygon", coords, std::forward<CNF>(cnf));
        }

        /// @brief Create rectangle with coordinates x1,y1,x2,y2.
        template<typename CNF = cnfs::Canvas_create_rectangle>
        long long create_rectangle(double x1, double y1, double x2, double y2, CNF&& cnf = {})
        {
            return this->_create("rectangle", { x1, y1, x2, y2 }, std::forward<CNF>(cnf));
        }

        /// @brief Create text with coordinates x ,y.
        template<typename CNF = cnfs::Canvas_create_text>
        long long create_text(double x, double y, CNF&& cnf = {})
        {
            return this->_create("text", { x, y }, std::forward<CNF>(cnf));
        }

        /// @brief Create window with coordinates x, y.
        template<typename CNF = cnfs::Canvas_create_window>
        long long create_window(double x, double y, CNF&& cnf = {})
        {
            return this->_create("window", { x, y }, std::forward<CNF>(cnf));
        }
    };

    /// @brief %Checkbutton widget which is either in on- or off-state.
    struct Checkbutton : Widget
    {
        friend BaseWidget;

    protected:
        template<typename Self>
        void _setup(this Self& self, const std::optional<Misc>& master_, auto& cnf, std::set<std::string>& ignore_fields)
        {
            //Because Checkbutton defaults to a variable with the same name as the widget, Checkbutton default names must be globally unique, not just unique within the parent widget.

            bool has_name = false;
            if constexpr (requires{ cnf.name; })
                utility::invoke_or_and_then([&](auto& val) { has_name = !val.empty(); }, cnf.name);

            if (!has_name)
            {
                auto name = hhh::misc::to_lower(reflect::type_name<Self>());
                detail::_checkbutton_count += 1;
                // To avoid collisions with ttk.Checkbutton, use the different name template.
                self.BaseWidget::_setup(master_, cnf, ignore_fields, std::format("!{}-{}", name, detail::_checkbutton_count));
            }
            else
                self.BaseWidget::_setup(master_, cnf, ignore_fields);
        }

    public:
        /// @brief Construct a new Button widget.
        CONSTRUCTORS_AND_ASSIGNMENT(Checkbutton, cnfs::Checkbutton<long long>, "checkbutton", Widget);

        /// @brief Put the button in off-state.
        void deselect()
        {
            this->tk->call(this->_w, "deselect");
        }

        /// @brief Flash the button.
        void flash()
        {
            this->tk->call(this->_w, "flash");
        }

        /// @brief Toggle the button and invoke a command if given as resource.
        void invoke()
        {
            return this->tk->call(this->_w, "invoke");
        }

        /// @brief Put the button in on-state.
        void select()
        {
            this->tk->call(this->_w, "select");
        }

        /// @brief Toggle the button.
        void toggle()
        {
            this->tk->call(this->_w, "toggle");
        }
    };

    /// @brief A checkbutton widget with a defined value type.
    ///
    /// Checkbutton has an arbitrary value type. TypedCheckbutton restricts the value to a specific type.
    /// @tparam T The value type.
    /// @see Checkbutton
    template<detail::AsObjConcept T>
    struct TypedCheckbutton : Checkbutton
    {
        using value_type = T;

        /// @brief Construct a new TypedCheckbutton widget.
        CONSTRUCTORS_AND_ASSIGNMENT(TypedCheckbutton, cnfs::Checkbutton<value_type>, "radiobutton", Checkbutton);
    };

    /// @brief %Entry widget which allows displaying simple text.
    struct Entry : Widget, XView<Entry>
    {
        /// @brief Construct a new Entry widget.
        CONSTRUCTORS_AND_ASSIGNMENT(Entry, cnfs::Entry, "entry", Widget);

        /// @brief Delete a character.
        void delete_(detail::index auto&& index)
        {
            this->tk->call(this->_w, "delete", detail::to_index(index));
        }
        /// @brief Delete text from FIRST to LAST (not included).
        void delete_(const detail::index auto& first, const detail::index auto& last)
        {
            this->tk->call(this->_w, "delete", detail::to_index(first), detail::to_index(last));
        }

        /// @brief Return the text.
        std::string get()
        {
            return this->tk->call<std::string>(this->_w, "get");
        }

        /// @brief Insert cursor at INDEX.
        void icursor(detail::index auto&& index)
        {
            this->tk->call(this->_w, "icursor", detail::to_index(index));
        }

        /// @brief Return position of cursor.
        long long index(detail::index auto&& index)
        {
            return this->tk->call<long long>(this->_w, "index", detail::to_index(index));
        }

        /// @brief Insert STRING at INDEX.
        void insert(detail::index auto&& index, const std::string& string)
        {
            this->tk->call(this->_w, "insert", detail::to_index(index), string);
        }

        /// @brief unknown
        void scan_mark(long long x)
        {
            this->tk->call(this->_w, "scan", "mark", x);
        }

        /// @brief unknown
        void scan_dragto(long long x)
        {
            this->tk->call(this->_w, "scan", "dragto", x);
        }

        /// @brief Adjust the end of the selection near the cursor to INDEX.
        void selection_adjust(detail::index auto&& index)
        {
            this->tk->call(this->_w, "selection", "adjust", detail::to_index(index));
        }

        /// @copydoc selection_adjust
        void select_adjust(detail::index auto&& index)
        {
            this->selection_adjust(detail::to_index(index));
        }

        /// @brief Clear the selection if it is in this widget.
        void selection_clear()
        {
            this->tk->call(this->_w, "selection", "clear");
        }

        /// @copydoc selection_clear
        void select_clear()
        {
            this->selection_clear();
        }

        /// @brief Set the fixed end of a selection to INDEX.
        void selection_from(detail::index auto&& index)
        {
            this->tk->call(this->_w, "selection", "from", detail::to_index(index));
        }

        /// @copydoc selection_from
        void select_from(detail::index auto&& index)
        {
            this->selection_from(detail::to_index(index));
        }

        /// @brief Return true if there are characters selected in the entry, false otherwise.
        bool selection_present()
        {
            return this->tk->call<bool>(this->_w, "selection", "present");
        }

        /// @copydoc selection_present
        bool select_present()
        {
            return this->selection_present();
        }

        /// @brief Set the selection from START to END (not included).
        void selection_range(const detail::index auto& start, const detail::index auto& end)
        {
            this->tk->call(this->_w, "selection", "range", detail::to_index(start), detail::to_index(end));
        }

        /// @copydoc selection_range
        void select_range(const detail::index auto& start, const detail::index auto& end)
        {
            this->selection_range(detail::to_index(start), detail::to_index(end));
        }

        /// @brief Set the variable end of a selection to INDEX.
        void selection_to(detail::index auto&& index)
        {
            this->tk->call(this->_w, "selection", "to", detail::to_index(index));
        }

        /// @copydoc selection_to
        void select_to(detail::index auto&& index)
        {
            this->selection_to(detail::to_index(index));
        }
    };

    /// @brief %Frame widget which may contain other widgets and can have a 3D border.
    struct Frame : Widget
    {
    protected:
        template<cnfs::is_cnf CNF>
        void _init_(const std::string& widgetName, CNF&& cnf)
        {
            std::vector<_cpptkinter::Tcl_Obj> extra{};
            std::set<std::string> ignore_fields{};
            if constexpr (requires { cnf.class_; })
            {
                utility::invoke_or_and_then([&]<typename T>(T && v) {
                    extra.emplace_back(_cpptkinter::AsObj("-class"));
                    extra.emplace_back(_cpptkinter::AsObj(std::forward<T>(v)));
                    ignore_fields.insert("class_");
                }, cnf.class_);
            }

            this->Widget::_init_(widgetName, std::forward<CNF>(cnf), std::move(extra), std::move(ignore_fields));
        }

    public:
        /// @brief Construct a frame widget.
        CONSTRUCTORS_AND_ASSIGNMENT(Frame, cnfs::Frame, "frame", Widget);
    };

    /// @brief %Label widget which can display text and bitmaps.
    struct Label : Widget
    {
        /// @brief Construct a label widget.
        CONSTRUCTORS_AND_ASSIGNMENT(Label, cnfs::Label, "label", Widget);
    };

    /// @brief %Listbox widget which can display a list of strings.
    struct Listbox : Widget
    {
        /// @brief Construct a listbox widget.
        CONSTRUCTORS_AND_ASSIGNMENT(Listbox, cnfs::Listbox, "listbox", Widget);

        /// @brief Activate item identified by INDEX.
        void activate(detail::index auto&& index)
        {
            this->tk->call(this->_w, "activate", detail::to_index(index));
        }

        /// @brief Return a tuple of X1,Y1,X2,Y2 coordinates for a rectangle which encloses the item identified by the given index.
        std::array<long long, 4> bbox(detail::index auto&& index)
        {
            return this->tk->call<std::array<long long, 4>>(this->_w, "bbox", detail::to_index(index));
        }

        /// @brief Return the indices of currently selected item.
        std::vector<long long> curselection()
        {
            return this->tk->call<std::vector<long long>>(this->_w, "curselection");
        }

        /// @brief Delete item at index.
        void delete_(detail::index auto&& index)
        {
            this->tk->call(this->_w, "delete", detail::to_index(index));
        }
        /// @brief Delete items from FIRST to LAST (included).
        void delete_(const detail::index auto& first, const detail::index auto& last)
        {
            this->tk->call(this->_w, "delete", detail::to_index(first), detail::to_index(last));
        }

        /// @brief Get the item at index.
        template<detail::FromObjConcept R>
        R get(detail::index auto&& index)
        {
            if constexpr (std::same_as<R, std::string>)
                return this->tk->call<R>(this->_w, "get", detail::to_index(index));
            else
            {
                auto res = this->tk->call<std::variant<R, std::string>>(this->_w, "get", detail::to_index(index));
                if (std::holds_alternative<R>(res))
                    return std::get<R>(res);
                else
                {
                    if (std::get<std::string>(res).empty())
                        throw utility::construct_exception<std::invalid_argument>(std::format("index {} was out of bounds", detail::to_index(index)));
                    else
                        throw utility::construct_exception<std::invalid_argument>(std::format("expected type {} but got std::string", reflect::type_name<R>()));
                }
            }
        }
        /// @brief Get list of items from FIRST to LAST (included).
        template<detail::FromObjConcept R>
        std::vector<R> get(const detail::index auto& first, const detail::index auto& last)
        {
            return this->tk->call<std::vector<R>>(this->_w, "get", detail::to_index(first), detail::to_index(last));
        }

        /// @brief Return index of item identified with INDEX.
        long long index(detail::index auto&& index)
        {
            return this->tk->call<long long>(this->_w, "index", detail::to_index(index));
        }

        /// @brief Insert ELEMENTS at INDEX.
        void insert(detail::index auto&& index, const detail::AsObjConcept auto&...elements)
        {
            this->tk->call(this->_w, "insert", detail::to_index(index), elements...);
        }
        /// @copydoc insert(const detail::index auto&, const detail::AsObjConcept auto&...)
        void insert(detail::index auto&& index, const detail::range_convertible_to_tcl_obj auto& elements)
        {
            this->tk->call(this->_w, "insert", detail::to_index(index), elements | std::views::transform(_cpptkinter::AsObj));
        }

        /// @brief Get index of item which is nearest to y coordinate Y.
        long long nearest(long long y)
        {
            return this->tk->call<long long>(this->_w, "nearest", y);
        }

        /// @brief unknown
        void scan_mark(long long x, long long y)
        {
            this->tk->call(this->_w, "scan", "mark", x, y);
        }

        /// @brief unknown
        void scan_dragto(long long x, long long y)
        {
            this->tk->call(this->_w, "scan", "dragto", x, y);
        }

        /// @brief Scroll such that INDEX is visible.
        void see(detail::index auto&& index)
        {
            this->tk->call(this->_w, "see", detail::to_index(index));
        }

        /// @brief Set the fixed end oft the selection to INDEX.
        void selection_anchor(detail::index auto&& index)
        {
            this->tk->call(this->_w, "selection", "anchor", detail::to_index(index));
        }

        /// @copydoc selection_anchor
        void select_anchor(detail::index auto&& index)
        {
            this->selection_anchor(detail::to_index(index));
        }

        /// @brief Clear the selection at index.
        void selection_clear(detail::index auto&& index)
        {
            this->tk->call(this->_w, "selection", "clear", detail::to_index(index));
        }
        /// @brief Clear the selection from FIRST to LAST (included).
        void selection_clear(const detail::index auto& first, const detail::index auto& last)
        {
            this->tk->call(this->_w, "selection", "clear", detail::to_index(first), detail::to_index(last));
        }

        /// @copydoc selection_clear(const detail::index auto&)
        void select_clear(detail::index auto&& index)
        {
            this->selection_clear(detail::to_index(index));
        }
        /// @copydoc selection_clear(const detail::index auto&, const detail::index auto&)
        void select_clear(const detail::index auto& first, const detail::index auto& last)
        {
            this->selection_clear(detail::to_index(first), detail::to_index(last));
        }

        /// @brief Return True if INDEX is part of the selection.
        bool selection_includes(detail::index auto&& index)
        {
            return this->tk->call<bool>(this->_w, "selection", "includes", detail::to_index(index));
        }

        /// @copydoc selection_includes
        bool select_includes(detail::index auto&& index)
        {
            return this->selection_includes(detail::to_index(index));
        }

        /// @brief Set the selection for index without changing the currently selected elements.
        void selection_set(detail::index auto&& index)
        {
            this->tk->call(this->_w, "selection", "set", detail::to_index(index));
        }
        /// @brief Set the selection from FIRST to LAST (included) without changing the currently selected elements.
        void selection_set(const detail::index auto& first, const detail::index auto& last)
        {
            this->tk->call(this->_w, "selection", "set", detail::to_index(first), detail::to_index(last));
        }

        /// @copydoc selection_set(const detail::index auto&)
        void select_set(detail::index auto&& index)
        {
            this->selection_set(detail::to_index(index));
        }
        /// @copydoc selection_set(const detail::index auto&, const detail::index auto&)
        void select_set(const detail::index auto& first, const detail::index auto& last)
        {
            this->selection_set(detail::to_index(first), detail::to_index(last));
        }

        /// @brief Return the number of elements in the listbox.
        long long size()
        {
            return this->tk->call<long long>(this->_w, "size");
        }

        /// @brief Return the resource value for an ITEM and an OPTION.
        template<detail::FromObjConcept R>
        void itemcget(detail::index auto&& index, const std::string& option)
        {
            return this->tk->call<R>(this->_w, "itemcget", detail::to_index(index), "-" + option);
        }

        /// @brief Get allowed keywords.
        std::map<std::string, std::array<std::string, 5>> itemconfigure(detail::index auto&& index)
        {
            std::string index_{};
            if constexpr (std::same_as<std::remove_cvref_t<decltype(detail::to_index(index))>, std::string>)
                index_ = detail::to_index(index);
            else
                index_ = std::to_string(detail::to_index(index));
            auto map = this->_configure({ "itemconfigure", index_ });

            auto key_view = map | std::views::keys;
            auto value_view = map | std::views::values | std::views::transform([](auto& arr) {
                std::array<std::string, 5> new_arr{};
                std::ranges::move(arr | std::views::transform([](auto& v) { return std::get<std::string>(v); }), new_arr.begin());
                return new_arr;
                });

            return std::views::zip(key_view, value_view) | std::ranges::to<std::map>();
        }
        /// @brief Configure resources of an ITEM.
        template<cnfs::is_cnf CNF = cnfs::Listbox_itemconfigure>
        void itemconfigure(detail::index auto&& index, CNF&& cnf = {})
        {
            auto&& index_ = detail::to_index(index);
            if constexpr (std::same_as<std::remove_cvref_t<decltype(index)>, std::string>)
                this->_configure({ "itemconfigure", index_ }, std::forward<CNF>(cnf));
            else
                this->_configure({ "itemconfigure", std::to_string(index_) }, std::forward<CNF>(cnf));
        }

        /// @copydoc itemconfigure(const detail::index auto&)
        std::map<std::string, std::array<std::string, 5>> itemconfig(detail::index auto&& index)
        {
            return this->itemconfigure(detail::to_index(index));
        }
        /// @copydoc itemconfigure(const detail::index auto&, CNF&&)
        template<cnfs::is_cnf CNF = cnfs::Listbox_itemconfigure>
        void itemconfig(detail::index auto&& index, CNF&& cnf = {})
        {
            this->itemconfigure(detail::to_index(index), std::forward<CNF>(cnf));
        }
    };

    /// @brief %Listbox widget with a defined element type.
    /// 
    /// Listbox has an arbitrary element type. TypedListbox restricts the element type to a specific type.
    /// @tparam T The element type.
    /// @see Listbox
    template<detail::FromObjConcept T>
    struct TypedListbox : Listbox
    {
        using value_type = T;

        /// @brief Construct a new TypedListbox widget.
        CONSTRUCTORS_AND_ASSIGNMENT(TypedListbox, cnfs::Listbox, "listbox", Listbox);

        /// @copydoc Listbox::get(const detail::index auto&)
        value_type get(detail::index auto&& index)
        {
            return this->Listbox::get<value_type>(detail::to_index(index));
        }
        /// @copydoc Listbox::get(const detail::index auto&, const detail::index auto&)
        std::vector<value_type> get(const detail::index auto& first, const detail::index auto& last)
        {
            return this->Listbox::get<value_type>(detail::to_index(first), detail::to_index(last));
        }

        /// @copydoc Listbox::insert(const detail::index auto&, const detail::AsObjConcept auto&...)
        void insert(detail::index auto&& index, const std::convertible_to<value_type> auto&...elements)
        {
            this->Listbox::insert(detail::to_index(index), static_cast<value_type>(elements)...);
        }
        /// @copydoc insert(const detail::index auto&, const detail::range_of_AsObj auto&)
        void insert(detail::index auto&& index, const utility::range_of_convertible_to<value_type> auto& elements)
        {
            this->Listbox::insert(detail::to_index(index), elements | std::views::transform([](auto& val) { return static_cast<value_type>(val); }));
        }
    };
}