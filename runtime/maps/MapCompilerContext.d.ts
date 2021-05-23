export interface Map {
}

export interface MapCompilerGetOptions {
    label: string;
    spawn_players?: boolean;
}

export interface MapCompilerContext {
    register(label: string, map: any): void;
    get(options: MapCompilerGetOptions): Map;
}

export interface MapCompilerContextMetaTable {
    create(this: void): MapCompilerContext;
}
export const MapCompilerContext: MapCompilerContextMetaTable;