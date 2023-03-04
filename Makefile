CXX := clang++
CXXFLAGS := -std=c++20 `sdl2-config --cflags`
CPPFLAGS := -MMD -MP
LDFLAGS := -lpthread `sdl2-config --libs`

define TARGET_template =
 SRCS_$(1) := $$(foreach name,$(2),src/$$(name).cc)
 OBJS_$(1) := $$(foreach name,$(2),build/$$(name).o)
 $(1): $$(OBJS_$(1))
	$(CXX) $(LDFLAGS) -o $(1) $$(OBJS_$(1))
endef

ALLUNITS := tetris fontedit fontpad fontclean fontdemo sdlctx font args text
ALLOBJS := $(ALLUNITS:%=build/%.o)
ALLDEPS := $(ALLUNITS:%=build/%.d)

TARGETS := tetris fontedit fontpad fontclean fontdemo
UNITS_tetris := tetris sdlctx font text args
UNITS_fontedit := fontedit font args sdlctx
UNITS_fontpad := fontpad font args
UNITS_fontclean := fontclean font args
UNITS_fontdemo := fontdemo sdlctx font text args

all: $(TARGETS)

$(foreach target,$(TARGETS),$(eval $(call TARGET_template,$(target),$(UNITS_$(target)))))

$(ALLOBJS): build/%.o: src/%.cc build/.dir
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

build/.dir:
	mkdir -p build
	touch build/.dir

.PHONY: clean
clean:
	rm -rf build

-include $(ALLDEPS)
