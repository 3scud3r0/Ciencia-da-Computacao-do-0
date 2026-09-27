from __future__ import annotations
import os,platform,shutil,subprocess
from dataclasses import dataclass,field

@dataclass
class ContainerSpec:
    command:list[str]
    hostname:str="cc0-container"
    env:dict[str,str]=field(default_factory=dict)
    memory_mb:int|None=None
    cpu_seconds:int|None=None

class ContainerRuntime:
    NAMESPACE_FLAGS=("--mount","--uts","--ipc","--pid","--fork","--mount-proc")
    def supported(self):
        return platform.system()=="Linux" and shutil.which("unshare") is not None
    def namespace_ids(self,pid="self"):
        base=f"/proc/{pid}/ns"
        if not os.path.isdir(base):return {}
        result={}
        for name in ("mnt","uts","ipc","pid","net","user"):
            path=os.path.join(base,name)
            if os.path.exists(path):result[name]=os.readlink(path)
        return result
    def command_for(self,spec:ContainerSpec):
        if not spec.command:raise ValueError("command vazia")
        if not self.supported():raise RuntimeError("requer Linux + util-linux unshare")
        shell='hostname "$CC0_HOSTNAME"; exec "$@"'
        return ["unshare",*self.NAMESPACE_FLAGS,"env","CC0_HOSTNAME="+spec.hostname,
                *[f"{k}={v}" for k,v in sorted(spec.env.items())],
                "sh","-c",shell,"sh",*spec.command]
    def run(self,spec:ContainerSpec,check=True):
        command=self.command_for(spec)
        def limits():
            import resource
            if spec.memory_mb is not None:
                cap=spec.memory_mb*1024*1024;resource.setrlimit(resource.RLIMIT_AS,(cap,cap))
            if spec.cpu_seconds is not None:
                resource.setrlimit(resource.RLIMIT_CPU,(spec.cpu_seconds,spec.cpu_seconds))
        return subprocess.run(command,check=check,capture_output=True,text=True,preexec_fn=limits)
