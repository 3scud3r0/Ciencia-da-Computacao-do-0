from dataclasses import dataclass,field
@dataclass
class File:
    data:str=""
@dataclass
class Directory:
    children:dict[str,object]=field(default_factory=dict)
class VirtualFS:
    def __init__(self):self.root=Directory()
    def _parts(self,path):
        if not path.startswith("/"):raise ValueError("use caminho absoluto")
        return [p for p in path.split("/") if p]
    def _parent(self,path,create=False):
        parts=self._parts(path)
        if not parts:raise ValueError("raiz não é arquivo")
        node=self.root
        for part in parts[:-1]:
            child=node.children.get(part)
            if child is None and create:
                child=Directory();node.children[part]=child
            if not isinstance(child,Directory):raise NotADirectoryError(part)
            node=child
        return node,parts[-1]
    def mkdir(self,path):
        parent,name=self._parent(path,True)
        current=parent.children.get(name)
        if current is None:parent.children[name]=Directory()
        elif not isinstance(current,Directory):raise FileExistsError(path)
    def write(self,path,data):
        parent,name=self._parent(path,True);parent.children[name]=File(str(data))
    def read(self,path):
        parent,name=self._parent(path);node=parent.children.get(name)
        if node is None:raise FileNotFoundError(path)
        if not isinstance(node,File):raise IsADirectoryError(path)
        return node.data
    def listdir(self,path="/"):
        node=self.root
        for part in self._parts(path):
            node=node.children.get(part)
            if node is None:raise FileNotFoundError(path)
            if not isinstance(node,Directory):raise NotADirectoryError(path)
        return sorted(node.children)
    def walk(self,path="/"):
        node=self.root
        for part in self._parts(path):
            node=node.children[part]
        out=[]
        def visit(prefix,current):
            if isinstance(current,File):
                out.append(prefix);return
            for name,child in sorted(current.children.items()):
                visit((prefix.rstrip("/")+"/"+name),child)
        visit(path,node);return out
