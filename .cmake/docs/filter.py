import os
import sys
import xml.etree.ElementTree as ET


def has_brief(elem: ET.Element) -> bool:
    brief = elem.find("briefdescription")
    return brief is not None and "".join(brief.itertext()).strip() != ""


def get_location(elem: ET.Element) -> str:
    loc = elem.find("location")
    if loc is not None:
        file_path = loc.get("file", "<unknown>")
        line = loc.get("line", "1")
        return f"{file_path}:{line}"
    return "<unknown>:1"


def filter_doxygen_xml(xml_dir: str) -> None:
    index_path = os.path.join(xml_dir, "index.xml")
    if not os.path.isfile(index_path):
        return

    index_tree = ET.parse(index_path)
    index_root = index_tree.getroot()

    # 1. Check all files for missing @file / @brief documentation
    for compound in index_root.findall("compound"):
        if compound.get("kind") == "file":
            refid = compound.get("refid", "")
            cpath = os.path.join(xml_dir, f"{refid}.xml")
            if os.path.isfile(cpath):
                cdef = ET.parse(cpath).getroot().find("compounddef")
                if cdef is not None and not has_brief(cdef):
                    name = cdef.findtext("compoundname", refid)
                    loc = get_location(cdef)
                    print(f"{loc}: warning: File '{name}' is not documented (missing /** @file ... @brief ... */)", file=sys.stderr)

    # 2. Collect all defined groups (kind="group") and the classes/structs inside them
    allowed_refids: set[str] = set()
    for compound in index_root.findall("compound"):
        if compound.get("kind") == "group":
            refid = compound.get("refid", "")
            allowed_refids.add(refid)
            group_xml = os.path.join(xml_dir, f"{refid}.xml")
            if os.path.isfile(group_xml):
                gtree = ET.parse(group_xml)
                for inner in gtree.findall(".//innerclass"):
                    inner_ref = inner.get("refid")
                    if inner_ref:
                        allowed_refids.add(inner_ref)
                for inner_grp in gtree.findall(".//innergroup"):
                    inner_ref = inner_grp.get("refid")
                    if inner_ref:
                        allowed_refids.add(inner_ref)

    # 3. Recursively expand nested classes/structs (<innerclass>) inside allowed classes/structs
    added = True
    while added:
        added = False
        for refid in list(allowed_refids):
            cpath = os.path.join(xml_dir, f"{refid}.xml")
            if os.path.isfile(cpath):
                ctree = ET.parse(cpath)
                for inner in ctree.findall(".//innerclass"):
                    inner_ref = inner.get("refid")
                    if inner_ref and inner_ref not in allowed_refids:
                        allowed_refids.add(inner_ref)
                        added = True

    # 4. Warn about classes/structs not in any @defgroup / @addtogroup / @ingroup
    for compound in index_root.findall("compound"):
        kind = compound.get("kind", "")
        refid = compound.get("refid", "")
        if kind in ("class", "struct") and refid not in allowed_refids:
            cpath = os.path.join(xml_dir, f"{refid}.xml")
            if os.path.isfile(cpath):
                cdef = ET.parse(cpath).getroot().find("compounddef")
                if cdef is not None:
                    name = cdef.findtext("compoundname", refid)
                    loc = get_location(cdef)
                    print(f"{loc}: warning: {kind.capitalize()} '{name}' is not part of any @defgroup / @addtogroup", file=sys.stderr)

    # 5. Verify grouped classes/structs actually have a @brief, and prune undocumented members
    verified_refids: set[str] = set()
    for refid in allowed_refids:
        cpath = os.path.join(xml_dir, f"{refid}.xml")
        if not os.path.isfile(cpath):
            continue
        ctree = ET.parse(cpath)
        croot = ctree.getroot()
        cdef = croot.find("compounddef")
        if cdef is None:
            continue

        # Keep all groups; for classes/structs, require an explicit @brief
        if cdef.get("kind") != "group" and not has_brief(cdef):
            name = cdef.findtext("compoundname", refid)
            loc = get_location(cdef)
            print(f"{loc}: warning: Grouped {cdef.get('kind')} '{name}' is missing @brief and will be excluded", file=sys.stderr)
            continue

        # Remove undocumented members (no @brief) from the compound
        for section in list(cdef.findall("sectiondef")):
            for member in list(section.findall("memberdef")):
                if not has_brief(member):
                    mname = member.findtext("name", "<anonymous>")
                    mloc = get_location(member)
                    print(f"{mloc}: warning: Member '{mname}' is not documented (missing @brief)", file=sys.stderr)
                    section.remove(member)
            if len(section.findall("memberdef")) == 0:
                cdef.remove(section)

        ctree.write(cpath, encoding="utf-8", xml_declaration=True)
        verified_refids.add(refid)

    # 6. Remove everything else from index.xml and delete unused .xml files
    for compound in list(index_root.findall("compound")):
        refid = compound.get("refid", "")
        if refid not in verified_refids:
            index_root.remove(compound)
            cpath = os.path.join(xml_dir, f"{refid}.xml")
            if os.path.isfile(cpath):
                os.remove(cpath)

    index_tree.write(index_path, encoding="utf-8", xml_declaration=True)


if __name__ == "__main__":
    filter_doxygen_xml(sys.argv[1])
