module;
#include "../global.hpp"
export module cpptkinter:cpptkinter.widget2;
import :constants;
import :utility;
import :_cpptkinter;
import :cpptkinter.detail;
import :cpptkinter.misc;
import :cpptkinter.cnfs;
import :cpptkinter.widget.base;
import :cpptkinter.widget1;
import std;


export namespace cpptkinter
{
    namespace cnfs
    {
        /// @brief Argument for Menubutton::Menubutton().
        struct Menubutton
        {
            opt_master master;
            opt_string activebackground;
            opt_string activeforeground;
            opt_anchor anchor;
            opt_string background;
            opt_screenunits bd;
            opt_string bg;
            opt_string bitmap;
            opt_screenunits border;
            opt_screenunits borderwidth;
            opt_compound compound;
            opt_cursor cursor;
            opt_string direction;
            opt_string disabledforeground;
            opt_string fg;
            opt_font_description font;
            opt_string foreground;
            opt_screenunits height;
            opt_string highlightbackground;
            opt_string highlightcolor;
            opt_screenunits highlightthickness;
            opt_image_spec image;
            opt_bool indicatoron;
            opt_string justify;
            opt_menu menu;
            opt_string name;
            opt_screenunits padx;
            opt_screenunits pady;
            opt_relief relief;
            opt_string state;
            opt_take_focus_value takefocus;
            opt_text text;
            opt_variable textvariable;
            opt<std::size_t> underline;
            opt_screenunits width;
            opt_screenunits wraplength;
        };
    }

    /// @brief %Menubutton widget, obsolete since Tk8.0.
    struct Menubutton : Widget
    {
        /// @brief Construct a menubutton widget.
        CONSTRUCTORS_AND_ASSIGNMENT(Menubutton, cnfs::Menubutton, "menubutton", Widget);
    };

    /// @brief %Message widget to display multiline text. Obsolete since Label does it too.
    struct [[deprecated("according to tkinter")]] Message;

    /// @brief %Radiobutton widget which shows only one of several buttons in on-state.
    struct Radiobutton : Widget
    {
        /// @brief Construct a radiobutton widget.
        CONSTRUCTORS_AND_ASSIGNMENT(Radiobutton, cnfs::Radiobutton<long long>, "radiobutton", Widget);

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
    };

    /// @brief A radiobutton widget with a defined value type.
    ///
    /// Radiobutton has an arbitrary value type. TypedRadiobutton restricts the value to a specific type.
    /// @tparam T The value type.
    /// @see Radiobutton
    template<detail::AsObjConcept T>
    struct TypedRadiobutton : Button
    {
        using value_type = T;

        /// @brief Construct a new TypedRadiobutton widget.
        CONSTRUCTORS_AND_ASSIGNMENT(TypedRadiobutton, cnfs::Radiobutton<T>, "radiobutton", Button);
    };

    /// @brief %Scale widget which can display a numerical scale.
    struct Scale : Widget
    {
        /// @brief Construct a scale widget.
        CONSTRUCTORS_AND_ASSIGNMENT(Scale, cnfs::Scale, "scale", Widget);

        /// @brief Get the current value as integer or float.
        double get()
        {
            return this->tk->call<double>(this->_w, "get");
        }

        /// @brief Set the current value.
        double set(double value)
        {
            return this->tk->call<double>(this->_w, "set", value);
        }

        /// @brief Return a tuple (X,Y) of the point along the centerline of the trough that corresponds to the current value.
        std::array<long long, 2> coords()
        {
            return this->tk->call<std::array<long long, 2>>(this->_w, "coords");
        }
        /// @brief Return a tuple (X,Y) of the point along the centerline of the trough that corresponds to VALUE.
        std::array<long long, 2> coords(double value)
        {
            return this->tk->call<std::array<long long, 2>>(this->_w, "coords", value);
        }

        /// @brief Return where the point X,Y lies. Valid return values are "slider", "though1" and "though2".
        std::string identify(detail::screenunits_arg auto&& x, detail::screenunits_arg auto&& y)
        {
            return this->tk->call<std::string>(this->_w, "identify", detail::to_screenunits_arg(x), detail::to_screenunits_arg(y));
        }
    };

    /// @brief %Scrollbar widget which displays a slider at a certain position.
    struct Scrollbar : Widget
    {
        /// @brief Construct a scrollbar widget.
        CONSTRUCTORS_AND_ASSIGNMENT(Scrollbar, cnfs::Scrollbar, "scrollbar", Widget);

        /// @brief Get the active element.
        /// 
        /// @returns The name of the element that is currently active, or "" if no element is active.
        std::string activate()
        {
            return this->tk->call<std::string>(this->_w, "activate");
        }
        /// @brief Marks the element indicated by index as active.
        /// 
        /// The only index values understood by this method are "arrow1", "slider", or "arrow2".
        /// If any other value is specified then no element of the scrollbar will be active.
        void activate(const std::string& index)
        {
            this->tk->call(this->_w, "activate", index);
        }

        /// @brief Return the fractional change of the scrollbar setting if it would be moved by DELTAX or DELTAY pixels.
        double delta(long long deltax, long long deltay)
        {
            return this->tk->call<double>(this->_w, "delta", deltax, deltay);
        }

        /// @brief Return the fractional value which corresponds to a slider position of X, Y.
        double fraction(long long x, long long y)
        {
            return this->tk->call<double>(this->_w, "fraction", x, y);
        }

        /// @brief Return the element under position X,Y as one of "arrow1", "slider", "arrow2" or "".
        std::string identify(long long x, long long y)
        {
            return this->tk->call<std::string>(this->_w, "identify", x, y);
        }

        /// @brief Return the current fractional values (upper and lower end) of the slider position.
        std::array<double, 2> get()
        {
            return this->tk->call<std::array<double, 2>>(this->_w, "get");
        }

        struct : utility::member_functor<impl>
        {
            using decays_to = void(const std::string&, const std::string&);

            void operator()(double first, double last)
            {
                self.tk->call(self._w, "set", first, last);
            }

            void operator()(const std::string& first, const std::string& last)
            {
                (*this)(std::stod(first), std::stod(last));
            }
        }
        /// @brief Set the fractional values of the slider position (upper and lower ends as value between 0 and 1).
        set{ *static_cast<impl*>(this->pimpl.get()) };
    };

    /// @brief %Text widget which can display text in various forms.
    struct Text : Widget, XView<Text>, YView<Text>
    {
        /// @brief Construct a text widget.
        CONSTRUCTORS_AND_ASSIGNMENT(Text, cnfs::Text, "text", Widget);

        /// @brief Return a tuple of (x, y, width, height) which gives the bounding box of the visible part of the character at the given index.
        std::array<long long, 4> bbox(detail::text_index auto&& index)
        {
            return this->tk->call<std::array<long long, 4>>(this->_w, "bbox", detail::to_text_index(index));
        }

        /// @brief Return whether between index INDEX1 and index INDEX2 the relation OP is satisfied.OP is one of <, <= , == , >= , >, or !=.
        bool compare(detail::text_index auto&& index1, const std::string& op, detail::text_index auto&& index2)
        {
            return this->tk->call<bool>(this->_w, "compare", detail::to_text_index(index1), op, detail::to_text_index(index2));
        }

        /// @brief Counts the number of relevant things between the two indices.
        /// 
        /// If index1 is after index2, the result will be a negative number (and this holds for each of the possible options).
        /// 
        /// The actual items which are counted depends on the options given by args.
        /// The result is a list of integers, one for the result of each counting option given.
        /// Valid counting options are "chars", "displaychars", "displayindices", "displaylines", "indices", "lines", "xpixels" and "ypixels".
        /// There is an additional possible  option "update", which if given then all subsequent options ensure that any possible out of date information is recalculated.
        template<std::convertible_to<std::string>...Args>
        std::vector<std::string> count(detail::text_index auto&& index1, detail::text_index auto&& index2, Args&&...args)
        {
            return this->tk->call<std::vector<std::string>>(this->_w, "count", (std::string("-") + std::forward<Args>(args))..., detail::to_text_index(index1), detail::to_text_index(index2));
        }

        /// @brief Get whether the internal consistency checks of the B-Tree inside the text widget is active.
        bool debug()
        {
            return this->tk->call<bool>(this->_w, "debug");
        }
        /// @brief Turn on the internal consistency checks of the B-Tree inside the text widget according to BOOLEAN.
        void debug(bool boolean)
        {
            this->tk->call(this->_w, "debug", boolean);
        }

        /// @brief Delete the character at INDEX.
        void delete_(detail::text_index auto&& index)
        {
            this->tk->call(this->_w, "delete", detail::to_text_index(index));

        }
        /// @brief Delete the characters between INDEX1 and INDEX2 (not included).
        void delete_(detail::text_index auto&& index1, detail::text_index auto&& index2)
        {
            this->tk->call(this->_w, "delete", detail::to_text_index(index1), detail::to_text_index(index2));
        }

        /// @brief Return tuple (x, y, width, height, baseline) giving the bounding box and baseline position of the visible part of the line containing the character at INDEX.
        std::array<long long, 5> dlineinfo(detail::text_index auto&& inde)
        {
            return this->tk->call<std::array<long long, 5>>(this->_w, "dlineinfo", detail::to_text_index(inde));
        }

        /// @brief Return the contents of the widget between index1 and index2.
        /// 
        /// The type of contents returned in filtered based on the keyword parameters;
        /// if 'all', 'image', 'mark', 'tag', 'text', or 'window' are given and true, then the corresponding items are returned.
        /// @returns A list of triples of the form (key, value, index). If none of the keywords are true then 'all' is used by default.
        template<cnfs::is_cnf CNF = cnfs::Text_dump>
        std::vector<std::array<std::string, 3>> dump(CNF&& cnf)
        {
            DEVIATING_IMPLEMENTATION_WARNING("original also allows passing a callback to receive the vector of arrays, we dont");
            std::vector<std::array<std::string, 3>> result{};
            std::vector<_cpptkinter::Tcl_Obj> args{};

            auto append_triple = [&](std::array<std::string, 3> arr) {
                result.emplace_back(std::move(arr));
                };
            auto func_name = this->_register(std::move(append_triple));
            args.emplace_back(_cpptkinter::AsObj("-command"));
            args.emplace_back(_cpptkinter::AsObj(func_name));
            if (cnf.all) args.emplace_back(_cpptkinter::AsObj("-all"));
            if (cnf.image) args.emplace_back(_cpptkinter::AsObj("-image"));
            if (cnf.mark) args.emplace_back(_cpptkinter::AsObj("-mark"));
            if (cnf.tag) args.emplace_back(_cpptkinter::AsObj("-tag"));
            if (cnf.text) args.emplace_back(_cpptkinter::AsObj("-text"));
            if (cnf.window) args.emplace_back(_cpptkinter::AsObj("-window"));
            args.emplace_back(_cpptkinter::AsObj(cnf.index1));
            if (cnf.index2.has_value())
                args.emplace_back(_cpptkinter::AsObj(cnf.index2.value()));
            this->tk->call(this->_w, "dump", args);
            return result;
        }

        /// @brief Internal method
        /// 
        /// This method controls the undo mechanism and the modified flag.
        /// The exact behavior of the command depends on the option argument that follows the edit argument.
        /// The following forms of the command are currently supported: edit_modified, edit_redo, edit_reset, edit_separator and edit_undo
        void edit(const detail::AsObjConcept auto&...args)
        {
            this->tk->call(this->_w, "edit", args...);
        }

        /// @brief Get the modified flag
        /// 
        /// The insert, delete, edit undo and edit redo commands or the user can set or clear the modified flag.
        /// @returns The modified flag of the widget.
        bool edit_modified()
        {
            return this->tk->call<bool>(this->_w, "edit", "modified");
        }
        /// @brief Set the modified flag
        /// 
        /// Sets the modified flag of the widget to arg.
        /// The insert, delete, edit undo and edit redo commands or the user can set or clear the modified flag.
        void edit_modified(bool arg)
        {
            this->tk->call(this->_w, "edit", "modified", arg);
        }

        /// @brief Redo the last undone edit
        /// 
        /// When the undo option is true, reapplies the last undone edits provided no other edits were done since then.
        /// Generates an error when the redo stack is empty.
        /// Does nothing when the undo option is false.
        void edit_redo()
        {
            this->tk->call(this->_w, "edit", "redo");
        }

        /// @brief Clears the undo and redo stacks.
        void edit_reset()
        {
            this->tk->call(this->_w, "edit", "reset");
        }

        /// @brief Inserts a separator (boundary) on the undo stack.
        /// 
        /// Does nothing when the undo option is false.
        void edit_separator()
        {
            this->tk->call(this->_w, "edit", "separator");
        }

        /// @brief Undoes the last edit action if the undo option is true.
        /// 
        /// An edit action is defined as all the insert and delete commands that are recorded on the undo stack in between two separators.
        /// Generates an error when the undo stack is empty.
        /// Does nothing when the undo option is false
        void edit_undo()
        {
            this->tk->call(this->_w, "edit", "undo");
        }

        /// @brief Return the text at INDEX.
        std::string get(detail::text_index auto&& index)
        {
            return this->tk->call<std::string>(this->_w, "get", detail::to_text_index(index));
        }
        /// @brief Return the text from INDEX1 to INDEX2 (not included).
        std::string get(detail::text_index auto&& index1, detail::text_index auto&& index2)
        {
            return this->tk->call<std::string>(this->_w, "get", detail::to_text_index(index1), detail::to_text_index(index2));
        }

        /// @brief Return the value of OPTION of an embedded image at INDEX.
        template<detail::FromObjConcept R>
        R image_cget(detail::text_index auto&& index, std::string option)
        {
            if (!option.starts_with('-'))
                option = '-' + option;
            if (option.ends_with('_'))
                option.pop_back();
            return this->tk->call<R>(this->_w, "image", "cget", detail::to_text_index(index), option);
        }

        /// @brief Configure an embedded image at INDEX.
        template<cnfs::is_cnf CNF = cnfs::Text_image_create>
        auto image_configure(CNF&& cnf)
        {
            return this->_configure({ "image", "configure", cnf.index }, this->_options(std::forward<CNF>(cnf), { "index" }));
        }

        /// @brief Create an embedded image at INDEX.
        template<cnfs::is_cnf CNF = cnfs::Text_image_create>
        void image_create(CNF&& cnf)
        {
            this->tk->call(this->_w, "image", "create", cnf.index, this->_options(std::forward<CNF>(cnf), { "index" }));
        }

        /// @brief Return all names of embedded images in this widget.
        std::vector<std::string> image_names()
        {
            return this->tk->call<std::vector<std::string>>(this->_w, "image", "names");
        }

        /// @brief Return the index in the form line.char for INDEX.
        std::string index(detail::text_index auto&& index)
        {
            return this->tk->call<std::string>(this->_w, "index", detail::to_text_index(index));
        }

        /// @brief Insert CHARS before the characters at INDEX. An additional tag can be given in ARGS. Additional CHARS and tags can follow in ARGS.
        void insert(detail::text_index auto&& index, const std::string& chars, auto&&...args)
        {
            this->tk->call(this->_w, "insert", detail::to_text_index(index), chars, args...);
        }

        /// @brief Get the gravity of a mark MARKNAME.
        std::string mark_gravity(const std::string& markName)
        {
            return this->tk->call<std::string>(this->_w, "mark", "gravity", markName);
        }
        /// @brief Change the gravity of a mark MARKNAME to DIRECTION (LEFT or RIGHT).
        void mark_gravity(const std::string& markName, const std::string& direction)
        {
            this->tk->call(this->_w, "mark", "gravity", markName, direction);
        }

        /// @brief Return all mark names.
        std::vector<std::string> mark_names()
        {
            return this->tk->call<std::vector<std::string>>(this->_w, "mark", "names");
        }

        /// @brief Set mark MARKNAME before the character at INDEX.
        void mark_set(const std::string& markName, detail::text_index auto&& index)
        {
            this->tk->call(this->_w, "mark", "set", markName, detail::to_text_index(index));
        }

        /// @brief 
        void mark_unset(std::convertible_to<std::string> auto&&...markNames)
        {
            this->tk->call(this->_w, "mark", "unset", std::string(std::forward<decltype(markNames)>(markNames))...);
        }

        /// @brief Return the name of the next mark after INDEX.
        std::string mark_next(detail::text_index auto&& index)
        {
            return this->tk->call<std::string>(this->_w, "mark", "next", detail::to_text_index(index));
        }

        /// @brief Return the name of the previous mark before INDEX.
        std::string mark_previous(detail::text_index auto&& index)
        {
            return this->tk->call<std::string>(this->_w, "mark", "previous", detail::to_text_index(index));
        }

        /// @brief Creates a peer text widget with the given newPathName, and any optional standard configuration options.
        /// 
        /// By default the peer will have the same start and end line as the parent widget, but these can be overridden with the standard configuration options.
        template<cnfs::is_cnf CNF = cnfs::Text_peer_create>
        void peer_create(CNF&& cnf)
        {
            this->tk->call(this->_w, "peer", "create", this->_options(std::forward<CNF>(cnf)));
        }

        /// @brief Returns a list of peers of this widget (this does not include the widget itself).
        std::vector<std::string> peer_names()
        {
            return this->tk->call<std::vector<std::string>>(this->_w, "peer", "names");
        }

        /// @brief Replaces the range of characters between index1 and index2 with the given characters and tags specified by args.
        /// 
        /// See the method insert for some more information about args, and the method delete for information about the indices.
        void replace(detail::text_index auto&& index1, detail::text_index auto&& index2, const std::string& chars, auto&&...args)
        {
            this->tk->call(this->_w, "replace", detail::to_text_index(index1), detail::to_text_index(index2), chars, args...);
        }

        /// @brief Remember the current X, Y coordinates.
        void scan_mark(long long x, long long y)
        {
            this->tk->call(this->_w, "scan", "mark", x, y);
        }

        /// @brief Adjust the view of the text to 10 times the difference between X and Y and the coordinates given in scan_mark.
        void scan_dragto(long long x, long long y)
        {
            this->tk->call(this->_w, "scan", "dragto", x, y);
        }

        /// @brief Search PATTERN beginning from INDEX until STOPINDEX.
        /// 
        // @returns The index of the first character of a match or an empty string.
        template<cnfs::is_cnf CNF = cnfs::Text_search>
        std::string search(CNF&& cnf)
        {
            std::vector<_cpptkinter::Tcl_Obj> args{ _cpptkinter::AsObj(this->_w), _cpptkinter::AsObj("search") };
            if (cnf.forwards) args.emplace_back(_cpptkinter::AsObj("-forwards"));
            if (cnf.backwards) args.emplace_back(_cpptkinter::AsObj("-backwards"));
            if (cnf.exact) args.emplace_back(_cpptkinter::AsObj("-exact"));
            if (cnf.regexp) args.emplace_back(_cpptkinter::AsObj("-regexp"));
            if (cnf.nocase) args.emplace_back(_cpptkinter::AsObj("-nocase"));
            if (cnf.elide) args.emplace_back(_cpptkinter::AsObj("-elide"));
            if (cnf.count.has_value()) { args.emplace_back(_cpptkinter::AsObj("-count")); _cpptkinter::AsObj(cnf.count.value()); }
            if (cnf.pattern.starts_with('-')) args.emplace_back(_cpptkinter::AsObj("--"));
            args.emplace_back(_cpptkinter::AsObj(cnf.pattern));
            args.emplace_back(_cpptkinter::AsObj(cnf.index));
            if (cnf.stopindex.has_value()) args.emplace_back(_cpptkinter::AsObj(cnf.stopindex.value()));
            return this->tk->call<std::string>(args);
        }

        /// @brief Scroll such that the character at INDEX is visible.
        void see(detail::text_index auto&& index)
        {
            this->tk->call(this->_w, "see", detail::to_text_index(index));
        }

        /// @brief Add tag TAGNAME to all characters between INDEX1 and index2 in ARGS. 
        /// 
        /// Additional pairs of indices may follow in ARGS.
        void tag_add(const std::string& tagName, detail::text_index auto&& index1, detail::text_index auto&&...args)
        {
            this->tk->call(this->_w, "tag", "add", tagName, detail::to_text_index(index1), detail::to_text_index(args)...);
        }

        /// @brief Unbind for all characters with TAGNAME for event SEQUENCE.
        void tag_unbind(const std::string& tagName, const std::string& sequence)
        {
            this->_unbind({ this->_w, "tag", "bind", tagName, sequence });
        }
        /// @brief Unbind for all characters with TAGNAME for event SEQUENCE the function identified with FUNCID.
        void tag_unbind(const std::string& tagName, const std::string& sequence, const std::string& funcid)
        {
            this->_unbind({ this->_w, "tag", "bind", tagName, sequence }, funcid);
        }

        /// @brief Bind to all characters with TAGNAME at event SEQUENCE a call to function FUNC.
        /// 
        /// An additional boolean parameter ADD specifies whether FUNC will be called additionally to the other bound function or whether it will replace the previous function.
        /// See bind for the return value.
        auto tag_bind(const std::string& tagName, const std::string& sequence, const std::string& func, bool add = false)
        {
            this->_bind({ this->_w, "tag", "bind", tagName }, sequence, func, add);
        }
        /// @copydoc tag_bind
        template<std::invocable<Event> Func>
        auto tag_bind(const std::string& tagName, const std::string& sequence, Func&& func, bool add = false)
        {
            this->_bind({ this->_w, "tag", "bind", tagName }, sequence, std::forward<Func>(func), add);
        }

        void _tag_bind();

        /// @brief Return the value of OPTION for tag TAGNAME.
        template<detail::FromObjConcept R>
        R cget(const std::string& tagName, std::string option)
        {
            if (!option.starts_with('-'))
                option = '-' + option;
            if (option.ends_with('_'))
                option.pop_back();
            return this->tk->call<R>(this->_w, "tag", "cget", tagName, option);
        }

        /// @brief Configure a tag TAGNAME.
        std::vector<std::array<std::string, 5>> tag_configure(const std::string& tagName)
        {
            return this->tk->call<std::vector<std::array<std::string, 5>>>(this->_w, "tag", "configure", tagName);
        }
        /// @brief Configure a tag TAGNAME.
        auto tag_configure(const std::string& tagName, const std::string& cnf)
        {
            return this->_configure({ "tag", "configure", tagName }, cnf);
        }
        /// @brief Configure a tag TAGNAME.
        template<cnfs::is_cnf CNF = cnfs::Text_tag_configure>
        void tag_configure(CNF&& cnf)
        {
            return this->_configure({ "tag", "configure", cnf.tagName }, std::forward<CNF>(cnf), { "tagName" });
        }

        /// @copydoc tag_configure(const std::string&)
        std::vector<std::array<std::string, 5>> tag_config(const std::string& tagName)
        {
            return this->tag_configure(tagName);
        }
        /// @copydoc tag_configure(const std::string&, const std::string&)
        auto tag_config(const std::string& tagName, const std::string& cnf)
        {
            return this->tag_configure(tagName, cnf);
        }
        /// @copydoc tag_configure(const std::string&, CNF&&)
        template<cnfs::is_cnf CNF = cnfs::Text_tag_configure>
        void tag_config(const std::string& tagName, CNF&& cnf)
        {
            return this->tag_configure(tagName, std::forward<CNF>(cnf));
        }

        /// @brief Delete all tags in TAGNAMES.
        void tag_delete(const std::string& first_tag_name, std::convertible_to<std::string> auto&&...tagNames)
        {
            this->tk->call(this->_w, "tag", "delete", first_tag_name, std::string(std::forward<decltype(tagNames)>(tagNames))...);
        }

        /// @brief Change the priority of tag TAGNAME such that it is lower than the priority of BELOWTHIS.
        void tag_lower(const std::string& tagName)
        {
            this->tk->call(this->_w, "tag", "lower", tagName);
        }
        /// @brief Change the priority of tag TAGNAME such that it is lower than the priority of BELOWTHIS.
        void tag_lower(const std::string& tagName, const std::string& belowThis)
        {
            this->tk->call(this->_w, "tag", "lower", tagName, belowThis);
        }

        /// @brief Return a list of all tag names.
        std::vector<std::string> tag_names()
        {
            return this->tk->call<std::vector<std::string>>(this->_w, "tag", "names");
        }
        /// @brief Return a list of all tag names.
        std::vector<std::string> tag_names(detail::text_index auto&& index)
        {
            return this->tk->call<std::vector<std::string>>(this->_w, "tag", "names", detail::to_text_index(index));
        }

        /// @brief Return a list of start and end index for the first sequence of characters starting at INDEX which all have tag TAGNAME.
        /// 
        /// The text is searched forward from INDEX.
        std::array<std::string, 2> tag_nextrange(const std::string& tagName, detail::text_index auto&& index)
        {
            return this->tk->call<std::array<std::string, 2>>(this->_w, "tag", "nextrange", tagName, detail::to_text_index(index));
        }
        /// @brief Return a list of start and end index for the first sequence of characters between INDEX1 and INDEX2 which all have tag TAGNAME.
        /// 
        /// The text is searched forward from INDEX1.
        std::array<std::string, 2> tag_nextrange(const std::string& tagName, detail::text_index auto&& index1, detail::text_index auto&& index2)
        {
            return this->tk->call<std::array<std::string, 2>>(this->_w, "tag", "nextrange", tagName, detail::to_text_index(index1), detail::to_text_index(index2));
        }

        /// @brief Return a list of start and end index for the first sequence of characters backwards from INDEX which all have tag TAGNAME.
        /// 
        /// The text is searched backwards from INDEX.
        std::array<std::string, 2> tag_prevrange(const std::string& tagName, detail::text_index auto&& index)
        {
            return this->tk->call<std::array<std::string, 2>>(this->_w, "tag", "prevrange", tagName, detail::to_text_index(index));
        }
        /// @brief Return a list of start and end index for the first sequence of characters between INDEX1 and INDEX2 which all have tag TAGNAME.
        /// 
        /// The text is searched backwards from INDEX1.
        std::array<std::string, 2> tag_prevrange(const std::string& tagName, detail::text_index auto&& index1, detail::text_index auto&& index2)
        {
            return this->tk->call<std::array<std::string, 2>>(this->_w, "tag", "prevrange", tagName, detail::to_text_index(index1), detail::to_text_index(index2));
        }

        /// @brief Change the priority of tag TAGNAME such that it is higher than the priority of ABOVETHIS.
        void tag_raise(const std::string& tagName)
        {
            this->tk->call(this->_w, "tag", "raise", tagName);
        }
        /// @brief Change the priority of tag TAGNAME such that it is higher than the priority of ABOVETHIS.
        void tag_raise(const std::string& tagName, const std::string& aboveThis)
        {
            this->tk->call(this->_w, "tag", "raise", tagName, aboveThis);
        }

        /// @brief Return a list of ranges of text which have tag TAGNAME.
        std::vector<std::array<std::string, 2>> tag_ranges(const std::string& tagName)
        {
            return this->tk->call<std::vector<std::array<std::string, 2>>>(this->_w, "tag", "ranges", tagName);
        }

        /// @brief Remove tag TAGNAME from the character at INDEX.
        void tag_remove(const std::string& tagName, detail::text_index auto&& index)
        {
            this->tk->call(this->_w, "tag", "remove", tagName, detail::to_text_index(index));
        }
        /// @brief Remove tag TAGNAME from the characters between INDEX1 and INDEX2.
        void tag_remove(const std::string& tagName, detail::text_index auto&& index1, detail::text_index auto&& index2)
        {
            this->tk->call(this->_w, "tag", "remove", tagName, detail::to_text_index(index1), detail::to_text_index(index2));
        }

        /// @brief Return the value of OPTION of an embedded window at INDEX.
        template<detail::FromObjConcept R>
        void window_cget(detail::text_index auto&& index, std::string option)
        {
            if (!option.starts_with('-'))
                option = '-' + option;
            if (option.ends_with('_'))
                option.pop_back();
            return this->tk->call<R>(this->_w, "window", "cget", detail::to_text_index(index), option);
        }

        /// @brief Configure an embedded window at INDEX.
        template<cnfs::is_cnf CNF = cnfs::Text_window_create>
        auto window_configure(CNF&& cnf)
        {
            return this->_configure({ "window", "configure", cnf.index }, this->_options(std::forward<CNF>(cnf), { "index" }));
        }

        /// @copydoc window_configure
        template<cnfs::is_cnf CNF = cnfs::Text_window_create>
        auto window_config(CNF&& cnf)
        {
            return this->window_configure(std::forward<CNF>(cnf));
        }

        /// @brief Create a window at INDEX.
        template<cnfs::is_cnf CNF = cnfs::Text_window_create>
        void window_create(CNF&& cnf)
        {
            this->tk->call(this->_w, "window", "create", cnf.index, this->_options(std::forward<CNF>(cnf), { "index" }));
        }

        /// @brief Return all names of embedded windows in this widget.
        std::vector<std::string> window_names()
        {
            return this->tk->call<std::vector<std::string>>(this->_w, "window", "names");
        }
    };

    namespace detail
    {
        /// @brief Internal class. It wraps the command in the widget OptionMenu.
        struct _setit
        {
            StringVar _var;
            std::string _value;
            std::function<void(const StringVar&)> _callback;

            _setit(StringVar var, std::string value, const std::function<void(const StringVar&)>& callback) : _var(std::move(var)), _value(std::move(value)), _callback(callback)
            {

            }

            void operator()()
            {
                this->_var.set(this->_value);
                if (this->_callback)
                    this->_callback(this->_var);
            }
        };
    }

    /// @brief %OptionMenu which allows the user to select a value from a menu.
    class OptionMenu : public Menubutton
    {
    protected:
        struct impl : Menubutton::impl
        {
            std::optional<Menu> _menu;
            std::string menuname;

            /// @brief Destroy this widget and the associated menu.
            void destroy() override
            {
                // keeps this from being destroyed before this function returns
                auto temp = this->shared_from_this();

                this->_menu.reset();

                this->BaseWidget::impl::destroy();
            }
        };

    private:
        REF_TO_IMPL(_menu);
    public:
        REF_TO_IMPL(menuname);

    protected:
        void _init_(const Misc& master, const StringVar& variable, const detail::sized_range_convertible_to_string auto& values, const std::function<void(const StringVar&)>& command)
        {
            if (values.size() == 0)
                throw utility::construct_exception<std::invalid_argument>("values must be non-empty");

            this->Menubutton::_init_("menubutton",
                cnfs::Menubutton{ .master = master, .anchor = "c", .borderwidth = 2, .highlightthickness = 2, .indicatoron = 1, .relief = constants::RAISED, .textvariable = variable });

            this->widgetName = "tk_optionMenu";
            auto&& menu = this->_menu.emplace(cnfs::Menu{ .master = *this, .name = "menu", .tearoff = 0 });
            this->menuname = menu._w;

            for (auto& v : values)
                menu.add_command({ .command = detail::_setit(variable, v, command), .label = v });

            (*this)["menu"] = menu;
        }

        DEFINE_IMPL_CONSTRUCTOR(OptionMenu, Menubutton);
    public:
        /// @brief Construct an optionmenu widget.
        OptionMenu(const Misc& master, const StringVar& variable, const detail::sized_range_convertible_to_string auto& values, const std::function<void(const StringVar&)>& command = {}) :
            OptionMenu(std::make_shared<impl>())
        {
            this->_init_(master, variable, values, command);
        }

        DEFINE_COPY_MOVE_CONSTRUCTORS_AND_ASSIGNMENT(OptionMenu);

        template<typename R>
            requires detail::FromObjConcept<R> || std::same_as<R, Menu> || std::same_as<R, std::optional<Menu>>
        R _getitem_(const std::string& name, std::type_identity<R>)
        {
            if (name == "menu")
            {
                if constexpr (std::same_as<R, Menu>)
                    return this->_menu.value();
                else if constexpr (std::same_as<R, std::optional<Menu>>)
                    return this->_menu;
            }
            else
            {
                if constexpr (detail::FromObjConcept<R>)
                    return this->Menubutton::_getitem_(name, std::type_identity<R>{});
            }

            throw utility::construct_exception<std::invalid_argument>(std::format("requested type {} not compatible with provided ressource name {}", reflect::type_name<R>, name));
        }
    };

    /// @brief %Widget which can display images in PGM, PPM, GIF, PNG format.
    struct PhotoImage : Image
    {
        using constructor_cnf = cnfs::PhotoImage;

        /// @brief Create an image with NAME.
        /// 
        /// Valid resource names : data, format, file, gamma, height, palette, width.
        template<typename CNF = constructor_cnf>
        PhotoImage(CNF&& cnf = {}) : Image("photo", std::forward<CNF>(cnf))
        {

        }

        /// @brief Display a transparent image.
        void blank()
        {
            this->tk->call(this->name, "blank");
        }

        template<detail::FromObjConcept R>
        R cget(const std::string& key)
        {
            return this->tk->call<R>(this->name, "cget", "-" + key);
        }

        template<detail::FromObjConcept R>
        R _getitem_(const std::string& key, std::type_identity<R>)
        {
            return this->tk->call<R>(this->name, "cget", "-" + key);
        }

        /// @brief Return a new PhotoImage with the same image as this widget.
        PhotoImage copy()
        {
            auto destImage = PhotoImage({ .master = this->tk });
            this->tk->call(destImage, "copy", this->name);
            return destImage;
        }

        /// @brief Return a new PhotoImage with the same image as this widget but zoom it with a factor of x in the X direction and y in the Y direction.
        /// 
        /// If y is not given, the default value is the same as x.
        PhotoImage zoom(long long x, long long y = std::numeric_limits<long long>::min())
        {
            auto destImage = PhotoImage({ .master = this->tk });
            if (y == std::numeric_limits<long long>::min())
                y = x;
            this->tk->call(destImage, "copy", this->name, "-zoom", x, y);
            return destImage;
        }

        /// @brief Return a new PhotoImage based on the same image as this widget but use only every Xth or Yth pixel.
        /// 
        /// If y is not given, the default value is the same as x.
        PhotoImage subsample(long long x, long long y = std::numeric_limits<long long>::min())
        {
            auto destImage = PhotoImage({ .master = this->tk });
            if (y == std::numeric_limits<long long>::min())
                y = x;
            this->tk->call(destImage, "copy", this->name, "-subsample", x, y);
            return destImage;
        }

        /// @brief Return the color (red, green, blue) of the pixel at X,Y.
        std::array<long long, 3> get(long long x, long long y)
        {
            return this->tk->call<std::array<long long, 3>>(this->name, "get", x, y);
        }

        /// @brief Put row formatted colors to image starting from position TO.
        /// 
        /// e.g. image.put("{red green} {blue yellow}", to = (4, 6))
        void put(detail::AsObjConcept auto&& data, std::optional<std::array<long long, 2>> to = std::nullopt)
        {
            std::vector<_cpptkinter::Tcl_Obj> args{ _cpptkinter::AsObj(this->name), _cpptkinter::AsObj("put"), _cpptkinter::AsObj(data) };
            if (to.has_value())
            {
                args.emplace_back(_cpptkinter::AsObj("-to"));
                args.emplace_back(_cpptkinter::AsObj(to.value()));
            }
            this->tk->call(args);
        }

        /// @brief 
        void write(const std::string& filename, const std::string& format = {}, std::optional<std::array<long long, 2>> from_coords = std::nullopt)
        {
            std::vector<_cpptkinter::Tcl_Obj> args{ _cpptkinter::AsObj(this->name), _cpptkinter::AsObj("write"), _cpptkinter::AsObj(filename) };
            if (!format.empty())
                args.emplace_back(_cpptkinter::AsObj(format));
            if (from_coords.has_value())
            {
                args.emplace_back(_cpptkinter::AsObj("-from"));
                args.emplace_back(_cpptkinter::AsObj(from_coords.value()));
            }
            this->tk->call(args);
        }

        /// @brief Return True if the pixel at x,y is transparent.
        void transparency_get(long long x, long long y)
        {
            this->tk->call<bool>(this->name, "transparency", "get", x, y);
        }

        /// @brief Set the transparency of the pixel at x,y.
        void transparency_set(long long x, long long y, bool boolean)
        {
            this->tk->call(this->name, "transparency", "set", x, y, boolean);
        }
    };

    /// @brief %Widget which can display images in XBM format.
    struct BitmapImage
    {
        using constructor_cnf = cnfs::BitmapImage;

        /// @brief Create a bitmap with NAME.
        /// 
        /// Valid resource names: background, data, file, foreground, maskdata, maskfile.
        template<typename CNF = constructor_cnf>
        BitmapImage(CNF&& cnf = {}) : Image("bitmap", std::forward<CNF>(cnf))
        {

        }
    };

    /// @brief %Spinbox widget.
    struct Spinbox : Widget
    {
        /// @brief Construct a spinbox widget.
        CONSTRUCTORS_AND_ASSIGNMENT(Spinbox, cnfs::Spinbox, "spinbox", Widget);

        /// @brief Return a tuple of X1,Y1,X2,Y2 coordinates for a rectangle which encloses the character given by index.
        /// 
        /// The first two elements of the list give the x and y coordinates of the upper - left corner of the screen area covered by the character (in pixels relative to the widget)
        /// and the last two elements give the width and height of the character, in pixels.
        /// The bounding box may refer to a region outside the visible area of the window.
        std::array<long long, 4> bbox(detail::index auto&& index)
        {
            return this->tk->call<std::array<long long, 4>>(this->_w, "bbox", detail::to_index(index));
        }

        /// @brief Delete one element of the spinbox.
        void delete_(detail::index auto&& index)
        {
            this->tk->call(this->_w, "delete", detail::to_index(index));
        }
        /// @brief Delete elements of the spinbox.
        ///
        /// First is the index of the first character to delete, and last is the index of the character just after the last one to delete.
        /// If last isn't specified it defaults to first + 1, i.e. a single character is deleted.
        void delete_(const detail::index auto& first, const detail::index auto& last)
        {
            this->tk->call(this->_w, "delete", detail::to_index(first), detail::to_index(last));
        }

        /// @brief Returns the spinbox's string.
        std::string get()
        {
            return this->tk->call<std::string>(this->_w, "get");
        }

        /// @brief Alter the position of the insertion cursor.
        /// 
        /// The insertion cursor will be displayed just before the character given by index.
        void icursor(detail::index auto&& index)
        {
            this->tk->call(this->_w, "icursor", detail::to_index(index));
        }

        /// @brief Returns the name of the widget at position x, y
        ///
        /// @returns none, buttondown, buttonup, entry
        std::string identify(long long x, long long y)
        {
            return this->tk->call<std::string>(this->_w, "identify", x, y);
        }

        /// @brief Returns the numerical index corresponding to index.
        long long index(detail::index auto&& index)
        {
            return this->tk->call<long long>(this->_w, "index", detail::to_index(index));
        }

        /// @brief Insert string s at index.
        void insert(detail::index auto&& index, const std::string& s)
        {
            this->tk->call(this->_w, "insert", detail::to_index(index), s);
        }

        /// @brief Causes the specified element to be invoked
        ///
        /// The element could be buttondown or buttonup triggering the action associated with it.
        void invoke(const std::string& element)
        {
            this->tk->call(this->_w, "invoke", element);
        }

        /// @brief Internal function.
        // void scan();

        /// @brief Records x and the current view in the spinbox window.
        /// 
        /// Used in conjunction with later scan dragto commands. Typically this command is associated with a mouse button press in the widget.
        void scan_mark(long long x)
        {
            this->tk->call(this->_w, "scan", "mark", x);
        }

        /// @brief Compute the difference between the given x argument and the x argument to the last scan mark command
        /// 
        /// It then adjusts the view left or right by 10 times the difference in x - coordinates.
        /// This command is typically associated with mouse motion events in the widget, to produce the effect of dragging the spinbox at high speed through the window.
        void scan_dragto(long long x)
        {
            this->tk->call(this->_w, "scan", "dragto", x);
        }

        /// @brief Internal function.
        // void selection();

        /// @brief Locate the end of the selection nearest to the character given by index, then adjust that end of the selection to be at index (i.e including but not going beyond index).
        /// 
        /// The other end of the selection is made the anchor point for future select to commands.
        /// If the selection isn't currently in the spinbox, then a new selection is created to include the characters between index and the most recent selection anchor point, inclusive.
        void selection_adjust(detail::index auto&& index)
        {
            this->tk->call(this->_w, "selection", "adjust", detail::to_index(index));
        }

        /// @brief Clear the selection
        /// 
        /// If the selection isn't in this widget then the command has no effect.
        void selection_clear()
        {
            this->tk->call(this->_w, "selection", "clear");
        }

        /// @brief Gets the currently selected element.
        std::string selection_element()
        {
            return this->tk->call<std::string>(this->_w, "selection", "element");
        }
        /// @brief Sets the currently selected element.
        /// 
        /// If a spinbutton element is specified, it will be displayed depressed.
        void selection_element(const std::string& element)
        {
            this->tk->call(this->_w, "selection", "element", element);
        }

        /// @brief Set the fixed end of a selection to INDEX.
        void selection_from(detail::index auto&& index)
        {
            this->tk->call(this->_w, "selection", "from", detail::to_index(index));
        }

        /// @brief Return true if there are characters selected in the spinbox, false otherwise.
        bool selection_present()
        {
            return this->tk->call<bool>(this->_w, "selection", "present");
        }

        /// @brief Set the selection from START to END (not included).
        void selection_range(const detail::index auto& start, const detail::index auto& end)
        {
            this->tk->call(this->_w, "selection", "range", detail::to_index(start), detail::to_index(end));
        }

        /// @brief Set the variable end of a selection to INDEX.
        void selection_to(detail::index auto&& index)
        {
            this->tk->call(this->_w, "selection", "to", detail::to_index(index));
        }
    };

    /// @brief %Labelframe widget.
    struct LabelFrame : Widget
    {
        /// @brief Construct a labelframe widget.
        CONSTRUCTORS_AND_ASSIGNMENT(LabelFrame, cnfs::LabelFrame, "labelframe", Widget);
    };

    namespace cnfs
    {
        /// @brief Argument for PanedWindow::add().
        struct PanedWindow_add
        {
            Widget child;
            opt<Widget> after;
            opt<Widget> before;
            opt_screenunits height;
            opt_screenunits minsize;
            opt_screenunits padx;
            opt_screenunits pady;
            opt_string style;
            opt_string stretch;
            opt_screenunits width;
        };

        /// @brief Argument for PanedWindow::paneconfigure().
        struct PanedWindow_paneconfigure
        {
            Widget tagOrId;
            opt<Widget> after;
            opt<Widget> before;
            opt_screenunits height;
            opt_screenunits minsize;
            opt_screenunits padx;
            opt_screenunits pady;
            opt_string style;
            opt_string stretch;
            opt_screenunits width;
        };
    }

    /// @brief %Panedwindow widget.
    struct PanedWindow : Widget
    {
        /// @brief Construct a panedwindow widget.
        CONSTRUCTORS_AND_ASSIGNMENT(PanedWindow, cnfs::PanedWindow, "panedwindow", Widget);

        /// @brief Add a child widget to the panedwindow in a new pane.
        /// 
        /// The child argument is the name of the child widget followed by pairs of arguments that specify how to manage the windows.
        /// The possible options and values are the ones accepted by the paneconfigure() method.
        template<cnfs::is_cnf CNF = cnfs::PanedWindow_add>
        void add(CNF&& cnf)
        {
            this->tk->call(this->_w, "add", std::forward<CNF>(cnf).child, this->_options(std::forward<CNF>(cnf), { "child" }));
        }

        /// @brief Remove the pane containing child from the panedwindow
        /// 
        /// All geometry management options for child will be forgotten.
        void remove(const std::derived_from<Widget> auto& child)
        {
            this->tk->call(this->_w, "forget", child);
        }

        /// @copydoc remove
        void forget(const std::derived_from<Widget> auto& child)
        {
            this->remove(child);
        }

        /// @brief Not implemented
        /// 
        /// Identify the panedwindow component at point x, y.
        /// 
        /// If the point is over a sash or a sash handle, the result is a two element list containing the index of the sash or handle, and a word indicating whether it is over a sash or a handle, such as { 0 sash } or {2 handle}.
        /// If the point is over any other part of the panedwindow, the result is an empty list.
        void identify(long long x, long long y);

        /// @brief 
        // void proxy();

        /// @brief Return the x and y pair of the most recent proxy location.
        std::array<long long, 2> proxy_coord()
        {
            return this->tk->call<std::array<long long, 2>>(this->_w, "proxy", "coord");
        }

        /// @brief Remove the proxy from the display.
        void proxy_forget()
        {
            this->tk->call(this->_w, "proxy", "forget");
        }

        /// @brief Place the proxy at the given x and y coordinates.
        void proxy_place(long long x, long long y)
        {
            this->tk->call(this->_w, "proxy", "place", x, y);
        }

        /// @brief 
        // void sash();

        /// @brief Return the current x and y pair for the sash given by index.
        /// 
        /// Index must be an integer between 0 and 1 less than the number of panes in the panedwindow.
        /// The coordinates given are those of the top left corner of the region containing the sash.
        /// pathName sash dragto index x y This command computes the difference between the given coordinates and the coordinates given to the last sash coord command for the given sash.
        /// It then moves that sash the computed difference.
        std::array<long long, 2> sash_coord(detail::index auto&& index)
        {
            return this->tk->call<std::array<long long, 2>>(this->_w, "sash", "coord", detail::to_index(index));
        }

        /// @brief Records x and y for the sash given by index.
        /// 
        /// Used in conjunction with later dragto commands to move the sash.
        void sash_mark(detail::index auto&& index)
        {
            this->tk->call(this->_w, "sash", "mark", detail::to_index(index));
        }

        /// @brief Place the sash given by index at the given coordinates.
        void sash_place(detail::index auto&& index, long long x, long long y)
        {
            this->tk->call(this->_w, "sash", "place", detail::to_index(index), x, y);
        }

        /// @brief Query a management option for window.
        /// 
        /// Option may be any value allowed by the paneconfigure subcommand.
        void panecget(const std::derived_from<Widget> auto& child, const std::string& option)
        {
            this->tk->call(this->_w, "panecget", child, "-" + option);
        }

        /// @brief Query or modify the management options for window.
        /// 
        /// If no option is specified, returns a list describing all of the available options for pathName.
        /// If option is specified with no value, then the command returns a list describing the one named option
        /// (this list will be identical to the corresponding sublist of the value returned if no option is specified).
        /// If one or more option - value pairs are specified, then the command modifies the given widget option(s) to have the given value(s);
        /// in this case the command returns an empty string. The following options are supported:
        /// 
        /// - <b>after window</b>: Insert the window after the window specified.window should be the name of a window already managed by pathName.
        /// - <b>before window</b>: Insert the window before the window specified. window should be the name of a window already managed by pathName.
        /// - <b>height size</b>: Specify a height for the window. The height will be the outer dimension of the window including its border, if any.
        /// If size is an empty string, or if - height is not specified, then the height requested internally by the window will be used initially;
        /// the height may later be adjusted by the movement of sashes in the panedwindow. Size may be any value accepted by Tk_GetPixels.
        /// - <b>minsize n</b>: Specifies that the size of the window cannot be made less than n.
        /// This constraint only affects the size of the widget in the paned dimension -- the x dimension for horizontal panedwindows, the y dimension for vertical panedwindows.
        /// May be any value accepted by Tk_GetPixels.
        /// - <b>padx n</b>: Specifies a non - negative value indicating how much extra space to leave on each side of the window in the X - direction.
        /// The value may have any of the forms accepted by Tk_GetPixels.
        /// - <b>pady n</b>: Specifies a non - negative value indicating how much extra space to leave on each side of the window in the Y - direction.
        /// The value may have any of the forms accepted by Tk_GetPixels.
        /// - <b>sticky style</b>: If a window's pane is larger than the requested dimensions of the window, this option may be used to position(or stretch) the window within its pane.
        /// Style is a string that contains zero or more of the characters n, s, e or w. The string can optionally contains spaces or commas, but they are ignored.
        /// Each letter refers to a side (north, south, east, or west) that the window will "stick" to.
        /// If both n and s (or e and w) are specified, the window will be stretched to fill the entire height (or width) of its cavity.
        /// - <b>stretch when</b>: Controls how extra space is allocated to each of the panes. When is one of always, first, last, middle, and never.
        /// The panedwindow will calculate the required size of all its panes. Any remaining (or deficit) space will be distributed to those panes marked for stretching.
        /// The space will be distributed based on each panes current ratio of the whole. The when values have the following definition:
        ///     - <b>always</b>: This pane will always stretch.
        ///     - <b>first</b>: Only if this pane is the first pane (left-most or top-most) will it stretch.
        ///     - <b>last</b>: Only if this pane is the last pane (right-most or bottom-most) will it stretch. This is the default value.
        ///     - <b>middle</b>: Only if this pane is not the first or last pane will it stretch.
        ///     - <b>never</b>: This pane will never stretch.
        /// - <b>width size</b>: Specify a width for the window.The width will be the outer dimension of the window including its border, if any.
        /// If size is an empty string, or if - width is not specified, then the width requested internally by the window will be used initially;
        /// the width may later be adjusted by the movement of sashes in the panedwindow. Size may be any value accepted by Tk_GetPixels.
        auto paneconfigure(const std::derived_from<Widget> auto& tagOrId) -> decltype(_getconfigure({}))
        {
            return this->_getconfigure({ _cpptkinter::AsObj(this->_w), _cpptkinter::AsObj("paneconfigure"), _cpptkinter::AsObj(tagOrId) });
        }
        /// @copydoc paneconfigure(const std::derived_from<Widget> auto&)
        auto paneconfigure(const std::derived_from<Widget> auto& tagOrId, const std::string& cnf) -> decltype(_getconfigure1({}))
        {
            return this->_getconfigure1({ _cpptkinter::AsObj(this->_w), _cpptkinter::AsObj("paneconfigure"), _cpptkinter::AsObj(tagOrId), _cpptkinter::AsObj("-" + cnf) });
        }
        /// @copydoc paneconfigure(const std::derived_from<Widget> auto&)
        template<cnfs::is_cnf CNF = cnfs::PanedWindow_paneconfigure>
        void paneconfigure(CNF&& cnf)
        {
            this->tk->call(this->_w, "paneconfigure", std::forward<CNF>(cnf).tagOrId, this->_options(std::forward<CNF>(cnf), { "tagOrId" }));
        }

        /// @copydoc paneconfigure(const std::derived_from<Widget> auto&)
        auto paneconfig(const std::derived_from<Widget> auto& tagOrId) -> decltype(paneconfigure(tagOrId))
        {
            return this->paneconfigure(tagOrId);
        }
        /// @copydoc paneconfigure(const std::derived_from<Widget> auto&, const std::string&)
        auto paneconfig(const std::derived_from<Widget> auto& tagOrId, const std::string& cnf) -> decltype(paneconfigure(tagOrId, cnf))
        {
            return this->paneconfigure(tagOrId, cnf);
        }
        /// @copydoc paneconfigure(CNF&&)
        template<cnfs::is_cnf CNF = cnfs::PanedWindow_paneconfigure>
        void paneconfig(CNF&& cnf)
        {
            this->paneconfigure(std::forward<CNF>(cnf));
        }

        /// @brief Returns an ordered list of the child panes.
        std::vector<_cpptkinter::Tcl_Obj> panes()
        {
            return this->tk->call<std::vector<_cpptkinter::Tcl_Obj>>(this->_w, "panes");
        }
    };
}